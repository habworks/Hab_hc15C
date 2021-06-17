/*****************************************************************
 *
 * File name:    	LPC2138_TIMER.C
 * Description:  	Functions used by the peripheral timer
 * Author:			Hab S. Collector
 * Date:			3/23/2011
 * Hardware:		
 * Firmware Tool:	CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
*****************************************************************/ 

// INCLUDE FILES
#include <TARGETS/LPC213X.H>                           // LPC21XX Peripheral Registers
#include <ctl_api.h>
#include <cross_studio_io.h>
#include "PSUPPLY_defines.h"


long volatile timeval = 0;                  // UPDATED EVERY 1ms
long volatile msCounter = 0;				// UPDATED EVERY 1ms
int volatile sCounter = 0;					// UPDATED EVERY 1s
int volatile TimerCounterA_100us = 0;		// INCREMENTED WITHIN IRQ TO MAKE 1ms
int volatile TimerCounterB_100us = 0;		// INCREMENTED WTTHIN IRQ TO MAKE 500ms
int volatile TimerCounterC_100us = 0;       // INCREMENTED WITHIN IRQ TO MAKE 1s
long volatile MeasureZeroX_100us = 0;       // INCREMENTED WITHIN IRQ TO MEASURE ZERO CROSS
int SwitchHoldTime_ms = 0;                  // INCREMENTED EVERY 1ms FOR USE WITH SWITCH DEBOUNCE
char volatile ControlData = ALL_OFF;
char volatile BlinkWarn = FALSE;


// EXTERNAL VARIABLES AND FUNCTIONS


// EXTERNAL FUNCTIONS USED HERE
extern void writeCTL_Byte(int);

// FUNCTIONS USED HERE
void timer1_IRQ_function(void);





/*************************************************************************
 * Function Name: ***ISR FOR: timer_counterMR0
 * Parameters: void
 * Return: void
 *
 * Description: IRQ handle - interrupts every 100us Counts 1ms and 500ms for delays and timeouts 
 * Every 1ms msCounter and timeval are incremented
 * Every 500ms ms500Counter is incremented
 *		
 *************************************************************************/
// Timer Counter 0 Interrupt executes each 1ms @ 12.000MHz PCLK Clock
// TIMEVAL INCREMENTED EVERY IRQ
void timer_counterMR0 (void) 
{
 
 // THESE VALUES ARE INCREMENTED ON A 100us INTERVAL
 TimerCounterA_100us++;
 TimerCounterB_100us++;
 TimerCounterC_100us++;
 MeasureZeroX_100us++;

 // TIMEVAL IS UPDATED EVERY 1ms
 if (TimerCounterA_100us > 9)
 	{
	timeval++;
	msCounter++;
    SwitchHoldTime_ms++;
	TimerCounterA_100us = 0;
	}

 /* TIMEVAL2 IS UPDATED EVERY 500ms
  * USED AS BLINK WARN.  NOTE: IT IS POSSIBLE FOR THIS IRQ (THE BLINK WARN) TO FIRE WHEN ANOTHER BUS
  * OPERATION IS IN PROGRESS.  AS SUCH THE BUS VALUE WILL BE IN CONTENTION.  QUALIFY THE
  * TIME VALUE WITH ALL ACTIVE BUS SIGNALS - IN THIS CASE READ_LSB & READ_SYS.
  */
 if ((TimerCounterB_100us > 4999) && (IO0PIN & READ_LSB) && (IO0PIN & READ_SYS))
 	{
    // CHECK IF WAITING_LED
    if (BlinkWarn == TRUE)
		{
		if (ControlData & BLINK_LED)
			ControlData &= ~BLINK_LED;
		else
			ControlData |= BLINK_LED;        
		// WRITE NEW BOOT BLINK STATUS
		writeCTL_Byte(ControlData);
		}
	TimerCounterB_100us = 0;
	}

// TIMEVAL IS UPDATED EVERY 1s
 if (TimerCounterC_100us > 9999)
 	{
	sCounter++;
	TimerCounterC_100us = 0;
	}

  // Acknowledge Interrupt
  T0IR = 1;
//  VICVectAddr = 0;		// FOR USE WHEN NOT USING THE TASKING LIB

}



/*************************************************************************
 * Function Name: init_timer0
 * Parameters: void
 * Return: void
 *
 * Description: init timer 0 for 100us
 *		
 *************************************************************************/
// IRQ SET TO RESET ITSELF, HIGHEST PRIORITY, AND ENABLED
void init_timer0(void) 
{
// TIMER 0
// Timer Counter 0 Interrupt executes each 100us @ 12.000MHz PCLK Clock
  T0MR0 = 1199;                                   // 100us =  1200 counts of PCLK - THIS IS IRQ INTERVAL
  T0MCR = 0x0003;                                 // Interrupt ON MR0 MR1 - RESET ON MR1
  T0TC = 0;						
  T0TCR = 1;                                      // Timer0 Enable
//  VICVectAddr0 = (unsigned long)timer_counterMR0; // set interrupt vector in 0
//  VICVectCntl0 = 0x20 | 4;                        // use it for Timer 0 Interrupt: TIMER 0 | IRQ SLOT ENABLE
//  VICIntSelect |= 0x00000010;                     // Enable Timer0 Interrupt
//  VICIntEnable |= 0x00000010;                     // Enable Timer0 Interrupt
  ctl_set_isr(TIMER0_IRQ, TIMER0_IRQ_PRIORITY, CTL_ISR_TRIGGER_FIXED, timer_counterMR0, 0);
  ctl_unmask_isr(TIMER0_IRQ);
}



/*************************************************************************
 * Function Name: init_timer1
 * Parameters: void
 * Return: void
 *
 * Description: init timer 1 for ValueIn_us match count
 * NOTE: Timer is init, but not enabled - MUST USED "ctl_unmask_isr(TIMER1_IRQ);" TO ENABLE
 *************************************************************************/
// IRQ SET TO RESET ITSELF, HIGHEST PRIORITY, AND ENABLED
void init_timer1(float ValueIn_us) 
{
// TIMER 0
// Timer Counter 1 Interrupt executes each 10us @ 12.000MHz PCLK Clock
  T1MR0 = (int)((12*ValueIn_us)+ROUNDING);          // For example 100us =  1200 counts of PCLK - THIS IS IRQ INTERVAL
  T1MCR = 0x0003;									// Interrupt ON MR0 MR1 - RESET ON MR1
  T1TCR = 1;										// Timer0 Enable
  T1TC = 0;
  T1IR = 1;
// USED WITH NOT CTL TASKS
//  VICVectAddr0 = (unsigned long)timer_counterMR0; // set interrupt vector in 0
//  VICVectCntl0 = 0x20 | 4;                        // use it for Timer 0 Interrupt: TIMER 0 | IRQ SLOT ENABLE
//  VICIntSelect |= 0x00000010;                     // Enable Timer0 Interrupt
//  VICIntEnable |= 0x00000010;                     // Enable Timer0 Interrupt
  ctl_set_isr(TIMER1_IRQ, TIMER1_IRQ_PRIORITY, CTL_ISR_TRIGGER_FIXED, timer1_IRQ_function, 0);
  //ctl_unmask_isr(TIMER1_IRQ);
}



/*************************************************************************
 * Function Name: ***ISR FOR: timer1_IRQ_function
 * Parameters: void
 * Return: void
 *
 * Description: IRQ handle - write function for what timer 1 does on IRQ here
 *
 *		
 *************************************************************************/
 void timer1_IRQ_function(void)
 {
	#ifdef DEBUG
		debug_printf("Problem Timer1 IRQ: Why was it called");
	#endif
 }





