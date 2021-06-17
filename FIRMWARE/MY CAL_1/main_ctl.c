 /**************************************************************************************************
 **************************PROJECT HEADER NOTES: YOU MUST READ THIS*********************************
 ***************************************************************************************************
 * PROJECT NAME: HC 15C 
 * PROJECT DESCRIPTION: APPLICATION FIRMWARE for Hab's HC15C Device a Scientific RPN (Reverse Polar Notation) 
 * based calculator with additional features like: volt and ohm meter, clock, alarm, SD Card Dir List, Music Play, etc.
 * The methodology for this calculator was developed and introduced in the Win App Hab's Next Cal and is (in
 * part) that methodology is used here.  For a complete overview of the firm consult the HC15 C Software
 * Description Document associated with this release.
 * The calculator will have the following features:
 *   1. Cap sense touch key
 *   2. 4x20 LCD display with ICONs and dimable back light
 *   3. A voltage and continuity meter
 *   4. A USB 2.0 compliant interface for use as a USB Mass Storage Device and Virtual COM
 *   5. A Serial to USB Converter Interface
 *   6. SD Memory Interface 4GB support FAT File System Support
 *   8. Audio support: for use with continuity meter
 *   9. Auto Selectable power sources (battery or USB(s))
 *   10. Application Support via USB Virtual COM to HabWorks MY CAL Win app for extended capability
 *   11. RTOS - Rowley Cross Works CTL environment 
 *   12. MP3 Player support 
 *
 * FIRMWARE ENGINEER:      HAB S. COLLECTOR
 * HARDWARE ENGINEER:      HAB S. COLLECTOR
 * SOFTWARE ENGINEER:      HAB S. COLLECTOR
 * PROJECT ENGINEER:       HAB S. COLLECTOR
 *
 * THIS FIRMWARE RESIDES ON THE TARGET HARDWARE: HC 15C
 * HC 15C SPECIFICS: 
 * HARDWARE PCB: PCB-HC15C REV2
 * HARDWARE SCH: SCH-HC15C REV2
 * 
 *
 * FIRMWARE DEVELOPMENT ENVIRONMENT: Rowley CrossStudio for ARM REV 2.1.1.2011..
 *
 * REVIEW MATERIAL: (TO BE ARCHEIVED WITH PROJECT)
 * REFERENCE DOCUMENTS: As this is firmware, Engineers reviewing this code may find it necessary to review both the hardware and software
 * documents associated with this file.  To complete your understanding consult reference documents for additional clarity:
 * HC15C User Manual and Technical Specification
 * HC15C Software Description Document
 * HC15C_REV2_Hardware (Schematic, PCB, BOM, document)  
 *
 *
 * ---------------------------------REVISION HISTORY----------------------------------------------
 * REV 1.0
 * ORGINAL RELEASE DATE: 8/3/12
 * NOTES:   Initial release to beta
 *
 * RELEASE: 1.1:    
 * NOTES:   First fix after beta
 * DATE:    8/20/12
 * BUG FIX: Calculator Mode, when in hex mode all values entered converted to 0 was fixed
 * BUG FIX: Calculator Mode, ClrX Key doing nothing was corrected.  Loaded values converted X register
            to 0, and unloaded values would be discarded.
 * BUG FIX: Calculator Mode, Tan(90) computed an incorrect result instead of giving an error.  Issue was
            Sin(x), Cos(x), and Tan(x) was not handling special values.  At some special values, the non-
            exact value of pi and computer rounding caused incorrect / non-expected returns.  These values
            were anticipated and correct / expected answers were forced. 
 * ADD NEW: Back light smooth turn on (ramp up) on POR
 * NOT FIX: On a force to deep sleep, or timeout to dream when deep sleep is entered and the external IRQ (SW2)
            or touch event wakes from deep sleep.  The events launched by wakeFromSleep will run, but the code
            will reboot itself the first time it tries to light sleep in idle mode after a deep sleep.
 **************************************************************************************************
 **************************************************************************************************/

#include <ctl_api.h>
#include <cross_studio_io.h>
#include <string.h>
#include "HC15C_DEFINES.H"
#include "CORE_FUNCTIONS.H"
#include "TIMERS_HC15C.H"
#include "CLOCK_TASKS.H"
#include "POWER_TASKS.H"
#include "TOUCH_TASKS.H"
#include "METER_TASKS.H"
#include "LIST_TASKS.H"
#include "AUDIO_TASKS.H"
#include "START_N_SLEEP_TASKS.H"
#include "SETUP_TASKS.H"
#include "USB_LINK.H"
#include "FAT_FS_INC/ff.h"
#define MEMORY_BLOCK_SIZE (sizeof(Type_AudioQueueStruct)/4+1)
#define MEMORY_BLOCK_COUNT 15

// GLOBALS
// FAT FS
FATFS fs[1];
FRESULT FF_Result;
FILINFO FF_Status;
DIR Directory;

// DIRECT TASKING ASSOCIATE DECLARATIONS
// TASKS
CTL_TASK_t main_task, 
           init_task,
           batQ_task,
           click_task,
           clock_task,
           meter_task,
           SDlist_task,
           audio_task,
           setup_task;

// TASKING EVENTS
CTL_EVENT_SET_t CalEvents;

// TASKING MESSAGE QUEUE
CTL_MESSAGE_QUEUE_t MsgQueue,
                    AudioQueue;
void *M_Queue[MAX_TOUCH_MSG];
void *A_Queue[MAX_AUDIO_MSG];

// MEMORY AREA FOR USE WITH THE AUDIO QUEUE
CTL_MEMORY_AREA_t MemArea;
unsigned memory[MEMORY_BLOCK_SIZE * MEMORY_BLOCK_COUNT]; 

// TASKING MUTEX
CTL_MUTEX_t ADC_Mutex,
            DelayMutex,
            DIP204Mutex;

// STACK DEFINITION
#define CALLSTACKSIZE 0 // FOR ARM BUILDS
#define STACKSIZE 200          
unsigned init_task_stack[1+ STACKSIZE +1],
         batQ_task_stack[1+ STACKSIZE +1],
         click_task_stack[1+ STACKSIZE +1],
         clock_task_stack[1+ STACKSIZE +1],
         meter_task_stack[1+ STACKSIZE +1],
         SDlist_task_stack[1+ (2*STACKSIZE) +1],
         audio_task_stack[1+ (4*STACKSIZE) +1],
         setup_task_stack[1+ STACKSIZE +1]; 


/*************************************************************************
 * Function Name: ctl_handle_error
 * Parameters: CTL_ERROR_CODE_t
 * Return: void
 *
 * Description: CTL RTOS error handler - Something in the CTL RTOS failed with no
 * recovery.  It is trapped here so you can inspect the parameter e.
 *************************************************************************/
void ctl_handle_error(CTL_ERROR_CODE_t e)
{
 while (1);
} // END OF FUNCTION ctl_handle_error




 /*************************************************************************
 * Function Name: main
 * Parameters:    void
 * Return:        int
 *
 * Description: Main - Init the RTOS control and scheduling functions, start tasks,
 * and establish main as the lowest priority active tasks.  Sleep as the idle task.
 * STEP 1: Init Msg Queues, Events, and Mutexs
 * STEP 2: Start main as a task and systick timer
 * STEP 3: De-Schedule tasks that are launched by events and that do not need to run all the time
 * STEP 4: Set main as the lowest priority task after all tasks have been created.  Launch
 * the Event_Init task
 * STEP 5: Sleep as main is the idle task - main is lowest priority tasks
 *************************************************************************/
int main(void)
{

  // STEP 1
  for (uint8_t Count = 0; Count < 20; Count++)
    {
    M_Queue[Count] = 0;
    }
  ctl_events_init(&CalEvents, 0);
  ctl_message_queue_init(&MsgQueue, M_Queue, 20);
  ctl_message_queue_init(&AudioQueue, A_Queue, MAX_AUDIO_MSG);
  ctl_memory_area_init(&MemArea, memory, MEMORY_BLOCK_SIZE, MEMORY_BLOCK_COUNT);
  ctl_mutex_init(&ADC_Mutex);
  ctl_mutex_init(&DelayMutex);
  ctl_mutex_init(&DIP204Mutex);
  
  // START MAIN AND SYSTICK
  ctl_task_init(&main_task, 255, "main"); // CREATE ADDITIONAL TASKS WHILE MAIN IS AT HIGHEST PRIORITY
  ctl_start_timer(SysTimerCallFromISR);   // START TIMER HERE
  
  // STEP 2
  // READY AND RUN POR init task
  memset(init_task_stack, 0xcd, sizeof(init_task_stack));  // INIT STACK AND WRITE KNOWN VALUE INTO STACK
  init_task_stack[0] = init_task_stack[(sizeof(init_task_stack)/sizeof(unsigned)) - 1] = 0xFaceFeed; // PLACE A MARKER VALUE AT START AND END OF STACK
  ctl_task_run(&init_task, 1, init_taskFn, 0, "init_task", (sizeof(init_task_stack)/sizeof(unsigned))-2, init_task_stack+1, CALLSTACKSIZE); // CREATE THE TASK
  
  // READY AND RUN battery charge task
  memset(batQ_task_stack, 0xcd, sizeof(batQ_task_stack));  
  batQ_task_stack[0] = batQ_task_stack[(sizeof(batQ_task_stack)/sizeof(unsigned)) - 1] = 0xFaceFeed; 
  ctl_task_run(&batQ_task, 2, batQ_taskFn, 0, "batQ_task", (sizeof(batQ_task_stack)/sizeof(unsigned))-2, batQ_task_stack+1, CALLSTACKSIZE);
    
  // READY AND RUN audio task
  memset(audio_task_stack, 0xcd, sizeof(audio_task_stack));  
  audio_task_stack[0] = audio_task_stack[(sizeof(audio_task_stack)/sizeof(unsigned)) - 1] = 0xFaceFeed; 
  ctl_task_run(&audio_task, 3, audio_taskFn, 0, "audio_task", (sizeof(audio_task_stack)/sizeof(unsigned))-2, audio_task_stack+1, CALLSTACKSIZE);
  
  // READY AND RUN setup task
  memset(setup_task_stack, 0xcd, sizeof(setup_task_stack));  
  setup_task_stack[0] = setup_task_stack[(sizeof(setup_task_stack)/sizeof(unsigned)) - 1] = 0xFaceFeed; 
  ctl_task_run(&setup_task, 4, setup_taskFn, 0, "setup_task", (sizeof(setup_task_stack)/sizeof(unsigned))-2, setup_task_stack+1, CALLSTACKSIZE);
    
  // READY AND RUN clock task
  memset(clock_task_stack, 0xcd, sizeof(clock_task_stack));  
  clock_task_stack[0] = clock_task_stack[(sizeof(clock_task_stack)/sizeof(unsigned)) - 1] = 0xFaceFeed; 
  ctl_task_run(&clock_task, 5, clock_taskFn, 0, "clock_task", (sizeof(clock_task_stack)/sizeof(unsigned))-2, clock_task_stack+1, CALLSTACKSIZE);
   
  // READY AND RUN meter task
  memset(meter_task_stack, 0xcd, sizeof(meter_task_stack));  
  meter_task_stack[0] = meter_task_stack[(sizeof(meter_task_stack)/sizeof(unsigned)) - 1] = 0xFaceFeed; 
  ctl_task_run(&meter_task, 6, meter_taskFn, 0, "meter_task", (sizeof(meter_task_stack)/sizeof(unsigned))-2, meter_task_stack+1, CALLSTACKSIZE);
    
  // READY AND RUN SD list task
  memset(SDlist_task_stack, 0xcd, sizeof(SDlist_task_stack));  
  SDlist_task_stack[0] = SDlist_task_stack[(sizeof(SDlist_task_stack)/sizeof(unsigned)) - 1] = 0xFaceFeed; 
  ctl_task_run(&SDlist_task, 7, SD_List_taskFn, 0, "SD_List_task", (sizeof(SDlist_task_stack)/sizeof(unsigned))-2, SDlist_task_stack+1, CALLSTACKSIZE);
  
  // READY AND RUN click task
  memset(click_task_stack, 0xcd, sizeof(click_task_stack));  
  click_task_stack[0] = click_task_stack[(sizeof(click_task_stack)/sizeof(unsigned)) - 1] = 0xFaceFeed; 
  ctl_task_run(&click_task, 8, click_taskFn, 0, "click_task", (sizeof(click_task_stack)/sizeof(unsigned))-2, click_task_stack+1, CALLSTACKSIZE);
  
  // STEP 3
  #if defined(REMOVE_RESTORE)
    ctl_task_remove(&clock_task);
    ctl_task_remove(&meter_task);
    ctl_task_remove(&SDlist_task);
  #elif defined(SUSPEND_RUN)
    ctl_HabTaskSuspend(&clock_task);
    ctl_HabTaskSuspend(&meter_task);
    ctl_HabTaskSuspend(&SDlist_task);
  #endif

  // STEP 4
  // DROP MAIN TO LOWEST PRIORITY TO ALLOW OTHER TASKS TO RUN
  ctl_task_set_priority(&main_task, 0);  
  ctl_events_set_clear(&CalEvents, EVENT_INIT, 0);
  
  // STEP 5
  // THIS IS THE IDLE TASK LOOP (MAIN IS THE IDEL TASK)
  while (1)
    {
    // SLEEP TO CONSERVER POWER WHEN DOING NOTHING: CORE CLK IF OFF BY ALL OTHER CLOCKS ON
    // ANY OF THE TOUCH IRQ'S, TIMER IRQs, OR RTC WILL WAKE UP
    CLKPWR_Sleep();
    }
  
  // CODE SHOULD NEVER REACH HERE
  return(0);

} // END OF main




/*************************************************************************
 * Function Name: check_failed
 * Parameters: uint8_t *, uint32_t
 * Return: void
 *
 * Description: Error trapping of the debug function - Necessary when in 
 * Debug mode
 *************************************************************************/
#ifdef DEBUG
  void check_failed(uint8_t *file, uint32_t line)
  {
  debug_printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  while(1);
  }
#endif