/******************************************************************************
 
 efsl Demo-Application for NXP LPC1768 Cortex-M3
 
 * @note
 * Copyright (C) 2010 NXP Semiconductors(NXP). 
 * All rights reserved.
 
 *****************************************************************************/

//***#include "Core_CM3/lpc17xx.h"
/*****#include "Terminal/monitor.h"*/
/*****#include "UART/lpc17xx_uart.h"*/

#include <cross_studio_io.h>
#include "HC15C_DEFINES.H"
#include "LED_HC15C.H"
#include "TIMER_HC15C.H"
#include "interfaces/lpc17xx_sd.h"
#include "efs.h"
#include "ls.h"




/* file name could be file.txt, /file.txt, dir1/file.txt, etc,
Note: does NOT support LFN */
#define FILE_NAME  "test07.txt"
//#define FILE_NAME  "/d1/d1_test1.txt"  // ok
#define FILE_RW_SIZE    (4*1024*1024)

#define READ_TEST_ENABLED  1    // 0 to disalbe
#define WRITE_TEST_ENABLED 1    // 0 to disalbe

EmbeddedFileSystem  efs;
EmbeddedFile filer, filew;
DirList             list;
uint8_t       file_name[13];


uint8_t Buff[1024];
uint32_t blen = sizeof(Buff);


volatile uint32_t Timer = 0;		/* Performance timer (1kHz increment) */

/* SysTick Interrupt Handler (1ms)    
void SysTick_Handler (void) 
{           
	static uint32_t div10;

	Timer++;

	if (++div10 >= 10) {
		div10 = 0;
		disk_timerproc();		// Disk timer function (100Hz)
	}
}
*/



#ifdef DEBUG
  void check_failed(uint8_t *file, uint32_t line)
  {
  debug_printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  while(1);
  }
#endif
 
  
// I added this as xmemset was in monitor.c Monitor.c is not used as I used debug_printf
void *xmemset (void *p, int c, int sz)
{
	register char *pf = (char*)p;

	while (sz--) *pf++ = (char)c;
	return p;
}





int main()
{
	int8_t res;
    uint32_t n, m, p, cnt;

// LEDS
    initLED();

// TIMER
ctl_HC15C_OnTimerCounter0(ISR_TimerCounter0);

//	SystemInit();

/*****    SysTick_Config(SystemCoreClock/1000 - 1); /* Generate interrupt each 1 ms   */
/*
	LPC17xx_UART_Init(115200); // UART0
    xfunc_out = LPC17xx_UART_PutChar;
	xfunc_in  = LPC17xx_UART_GetChar; 
*/
	debug_printf("\nMMC/SD Card Filesystem Test (P:LPC1768 L:EFSL)\n");

	debug_printf("\nCARD init...");

	// init file system
	if ( ( res = efs_init( &efs, 0 ) ) != 0 ) {
		debug_printf("failed with %i\n",res);
	}
	else 
	{
		debug_printf("ok\n");

        debug_printf("Card type: ");
        switch (CardType)
        {
            case CARDTYPE_MMC:
                debug_printf("MMC\n");
                break;
            case CARDTYPE_SDV1:
                debug_printf("Version 1.x Standard Capacity SD card.\n");
                break;
            case CARDTYPE_SDV2_SC:
                debug_printf("Version 2.0 or later Standard Capacity SD card.\n");
                break;
            case CARDTYPE_SDV2_HC:
                debug_printf("Version 2.0 or later High/eXtended Capacity SD card.\n");
                break;
            default:
                break;            
        }
        debug_printf("Sector size: %d bytes\n", CardConfig.sectorsize);
        debug_printf("Sector count: %d\n", CardConfig.sectorcnt);
        debug_printf("Block size: %d sectors\n", CardConfig.blocksize);
        debug_printf("Card capacity: %d MByte\n\n", (((CardConfig.sectorcnt >> 10) * CardConfig.sectorsize)) >> 10);

		debug_printf("\nDirectory of 'root':\n");
		
		/* list files in root directory */
		ls_openDir( &list, &(efs.myFs) , "/");
		while ( ls_getNext( &list ) == 0 ) {
			// list.currentEntry is the current file
			list.currentEntry.FileName[LIST_MAXLENFILENAME-1] = '\0';
			debug_printf("%s, 0x%x bytes\n", list.currentEntry.FileName, list.currentEntry.FileSize ) ;
		}

#if WRITE_TEST_ENABLED!=0
        /* Write test*/  
        debug_printf("\nFile write test:\n");
        debug_printf("Open file %s ...", FILE_NAME);
        if (file_fopen( &filew, &efs.myFs , FILE_NAME , 'a' ) == 0 )
        {
            debug_printf(" OK. \nWriting %lu bytes ...\n", FILE_RW_SIZE);

            xmemset(Buff, 0xAA, blen);
            n=FILE_RW_SIZE;
            m = 0;
            Timer = 0;
            while (n)
            {
                if (n>=blen) {
                    cnt = blen;
                    n -= blen;
                } else {
                    cnt = n;
                    n = 0;
                }
                p = file_write( &filew, cnt, Buff );
                m += p;
                if (p != cnt) break;
            }
            debug_printf("%lu bytes written with %lu kB/sec.\n", m, Timer ? (m / Timer) : 0);

            file_fclose( &filew );                          

        } else {
            debug_printf (" Failed.\n");
        }
#endif

#if READ_TEST_ENABLED!=0
        /* Read test */
        debug_printf("\nFile read test:\n");
        debug_printf("Open file %s ...", FILE_NAME);
        if (file_fopen( &filer, &efs.myFs , FILE_NAME , 'r' ) == 0 )
        {
            debug_printf(" OK. \nReading %lu bytes ...\n", FILE_RW_SIZE);

            n=FILE_RW_SIZE; 
            m = 0;
            Timer = 0;
            while (n)
            {
                if (n>=blen) {cnt = blen; n -= blen;}
                else         {cnt = n; n = 0;}

                p =  file_read( &filer, cnt, Buff );
                m += p;
                if (p != cnt) break;                
            }
            debug_printf("%lu bytes read with %lu kB/sec.\n", m, Timer ? (m / Timer) : 0);

            file_fclose( &filer ); 

        } else
        {
            debug_printf (" Failed.\n");    
        }
#endif

        /* close file system */
	    fs_umount( &efs.myFs ) ;
    }

	debug_printf("\nEFSL test complete.\n");

	while (1);
}
