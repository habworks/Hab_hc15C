/*****************************************************************
 *
 * File name:       FAT_TIME.C
 * Description:     Functions used to support the FAT FS File System
 * Author:          Hab S. Collector
 * Date:            2/16/12
 * Hardware:        NXP LPC1768
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           Part of the FAT FS - FAT Time for file system
 *****************************************************************/ 

#include "integer.h"
#include "lpc17xx_rtc.h"


/*************************************************************************
 * Function Name: get_fattime
 * Parameters:    void
 * Return:        DWORD
 *
 * Description: Returns the time to the FAT FS file system.  This function is
 * only called by FAT FS functions.  It is a necessary part of what I have to provide
 * STEP 1: Return the time to a DWORD as formated below.  The bit locations are
 * determined by the FAT FS documentation
 *************************************************************************/
DWORD get_fattime(void)
 {
  
  uint32_t Time, Year, Month, DayOfMonth, Hour, Minute, Second;
  
  // STEP 1
  Year = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_YEAR) << 25;
  Month = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MONTH) << 21;
  DayOfMonth = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_DAYOFMONTH) << 16;
  Hour = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR) << 11;
  Minute = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE) << 5;
  Second = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND)/2;
  
  Time = (Year|Month|DayOfMonth|Hour|Minute|Second);
  return(Time);
  
 } // END OF get_fattime