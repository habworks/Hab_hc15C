 /**************************************************************************************************
 **************************PROJECT HEADER NOTES: YOU MUST READ THIS*********************************
 ***************************************************************************************************
 * PROJECT NAME: HC 15C TEST HARDWARE
 * PROJECT DESCRIPTION: A collection of routines to test hardware of HC 15C REV 1
 *
 * FIRMWARE ENGINEER:      HAB S. COLLECTOR
 * HARDWARE ENGINEER:      HAB S. COLLECTOR
 * PROJECT ENGINEER:       HAB S. COLLECTOR
 * ORGINAL RELEASE DATE: 7/3/11
 * RELEASE: 1, 
 *
 * THIS FIRMWARE RESIDES ON THE TARGET HARDWARE
 * EUT SPECIFICS: 
 * EUT HARDWARE PCB: PCB-HC15C
 * EUT HARDWARE SCH: SCH-HC15C
 * 
 *
 * FIRMWARE DEVELOPMENT ENVIRONMENT: Rowley CrossStudio for ARM REV 2.0.3...
 *
 * REVIEW MATERIAL: (TO BE ARCHEIVED WITH PROJECT)
 * REFERENCE DOCUMENTS: As this is firmware, Engineers reviewing this code may find it necessary to review both the hardware and software
 * documents associated with this file.  To complete your understanding consult reference documents for additional clarity:
 *               
 **************************************************************************************************
 **************************************************************************************************/


// INCLUDE FILES
#include <stdio.h>
#include <ctl_api.h>
#include "CMSIS\lpc17xx.h"

//#include "CMSIS\lpc_types.h"
#include <cross_studio_io.h>
#include "HC15C_DEFINES.H"
#include "TIMER_HC15C.H"
#include "PWM_HC15C.H"
#include "ADC_HC15C.H"
#include "DIP204.H"
#include "CAT24C02.H"
#include "CP2101_HC15C.H"
#include "STMPE24M31_HC15C.H"

#include "CMSIS\lpc17xx_clkpwr.h"
// INCLUDED FOR V-COM USB
#include "VCOM\usbSerial.h"




#define RA                (300.0E3 + 10.0E3)
#define RC                100.0
#define RD                (7.5E3 + 250.0)



extern void initLED(void);
extern void switchLED (uint8_t LED_Status, uint32_t LEDs_ToChange);
extern void delayXms(uint32_t);
extern volatile uint32_t ACAP_TouchKey, BCAP_TouchKey;
extern volatile uint8_t KeyPressed_A, KeyPressed_B;


#ifdef DEBUG
  void check_failed(uint8_t *file, uint32_t line)
  {
  debug_printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  while(1);
  }
#endif


// VARIABLES DEFINED HERE USED IN THIS MODULE AND OTHERS


// VARIABLES USED HERE DEFINED ELSEWHERE


// PROTOTYPE EXTERNAL FUNCTIONS




/*************************************************************************
 * Function Name: main
 * Parameters: void
 * Return: int
 *
 * Description: MAIN ROUTINE: THIS FIRMWARE USES A TASKING LIBRARY FUNCTIONS
 * FOR IRQ SETUPS ONLY.  THE USE OF THE TASKING FUNCTIONS DOES NOT MAKE THIS
 * AN RTOS BASED SOFTWARE.  THE SOFTWARE IS VERY PROCEDURALY IN NATURE.  IT
 * BASICALY RUNS ONE TEST AFTER ANOTHER
 * STEP 1 Define GPIO, set default states, init hardware resources
 * STEP 2 Show intro test screen 
 * STEP 3 Enter endless loop to run various tests on the EUT.  In the event of a failed
 * test - report as a fail and turn off.  If all tests pass then report as passed.
 * NOTES: 
 *      1. Fail and pass conditions are both handled by test functions
 *************************************************************************/
int main(void)
{
  // DEFINE VARS
  uint8_t LineText[20];
  uint32_t PCLK_Value;
  double MeasuredResistance, I1, I2, Vb;
  // DEFINE TYPES
  PWM_HC15C_Type PWM_HC15C_Struct;
  ADC_HC15C_Type ADC_HC15C_Struct;
  
  
    // DEFINE LOCAL VARIABLES

    // PLAY WITH TASK TIMER 0
    ctl_HC15C_OnTimerCounter0(ISR_TimerCounter0);

    

    // PLAY WITH LEDs
    // DEFINE LEDS
    initLED();
    switchLED(LED_ON, (LED_USB|LED_DRIVE|LED_METER|LED_HEX|LED_RAD));
    delayXms(500);
    switchLED(LED_OFF, (LED_USB|LED_DRIVE|LED_METER|LED_HEX|LED_RAD));
    delayXms(200);
    switchLED(LED_ON, LED_USB);
    delayXms(200);
    switchLED(LED_ON, (LED_USB|LED_DRIVE));
    delayXms(200);
    switchLED(LED_ON, (LED_USB|LED_DRIVE|LED_METER));
    delayXms(200);
    switchLED(LED_ON, (LED_USB|LED_DRIVE|LED_METER|LED_HEX));
    delayXms(200);
    switchLED(LED_ON, (LED_USB|LED_DRIVE|LED_METER|LED_HEX|LED_RAD));
    delayXms(200);
    switchLED(LED_OFF, (LED_USB|LED_DRIVE|LED_METER|LED_HEX|LED_RAD));
    delayXms(200);
    switchLED(LED_ON, LED_DRIVE);

  // PLAY WITH DISPLAY
    init_DIP204();

// PWM PLAY
GPIO_SetDir(PORT0,(AOUT),1);
GPIO_ClearValue(PORT0, AOUT);

PWM_PWMToTimerMode();
// BACKLIGHT
PWM_HC15C_Struct.PWM_Frequency = 1000;
PWM_HC15C_Struct.PWM_DutyCycle = 30;
PWM_HC15C_Struct.PWM_Type = PWM_BACK_LIGHT;
init_PWM(PWM_HC15C_Struct);

// TONE
PWM_HC15C_Struct.PWM_Type = PWM_AUDIO_TONE;
init_PWM(PWM_HC15C_Struct);
// STOP BOTH
PWM_stop(PWM_AUDIO_TONE);
PWM_stop(PWM_BACK_LIGHT);
// START BOTH
PWM_HC15C_Struct.PWM_Type = PWM_AUDIO_TONE;
PWM_HC15C_Struct.PWM_DutyCycle = 10;
init_PWM(PWM_HC15C_Struct);
PWM_start(PWM_AUDIO_TONE);
PWM_start(PWM_BACK_LIGHT);
delayXms(500);
PWM_stop(PWM_AUDIO_TONE);
PWM_stop(PWM_BACK_LIGHT);
PWM_HC15C_Struct.PWM_DutyCycle = 50;
init_PWM(PWM_HC15C_Struct);
PWM_start(PWM_AUDIO_TONE);
PWM_start(PWM_BACK_LIGHT);
delayXms(500);
PWM_stop(PWM_AUDIO_TONE);
PWM_stop(PWM_BACK_LIGHT);
PWM_HC15C_Struct.PWM_DutyCycle = 90;
init_PWM(PWM_HC15C_Struct);
PWM_stop(PWM_AUDIO_TONE);
PWM_start(PWM_BACK_LIGHT);

/* PLAY WITH USB MEM
uint32_t n;

  // SystemClockUpdate() updates the SystemFrequency variable
  SystemClockUpdate();

  for (n = 0; n < MSC_ImageSize; n++) {     // Copy Initial Disk Image 
    Memory[n] = DiskImage[n];               //   from Flash to RAM     
  }

  USB_Init();                               // USB Initialization 
  USB_Connect(TRUE);                        // USB Connect 

//  while (1);                                // Loop forever 
*/

/* PLAY USB VCOM
// THIS IS NOT WORKING - NOT SURE WHY, BUT THE VCOM STUFF WORKS
// IS WORKING IN HC15C.  I WAS PLAYING WITH THIS AND BROKE SOMETHING.
uint32_t i;
uint8_t GetChar;
usbSerialInit();
    while(1)
    {
        VCOM_printf("\nLooping %d times. Press a key...",i);
        GetChar = VCOM_getc_echo();
        if (GetChar == 'x')
           {
           USBHwConnect(FALSE);
           break;
           }
        i++;
    }
*/


/* ADC PLAY 
init_ADC_polling();
ADC_HC15C_Struct.ADC_AvgWeight = 16;
//GPIO_SetValue(PORT1, (RANGE_30V|SEL_V_C));
GPIO_ClearValue(PORT1, (RANGE_10V|RANGE_10V|SEL_V_C));
GPIO_SetValue(PORT1, (RANGE_30V));

for(;;)//(uint8_t I=0;I<10;I++)
  {
  delayXms(100);
  ADC_HC15C_Struct.ADC_Type = ADC_MET_VOLTAGE;
  ADC_HC15C_Struct.ADC_FrontEndDivider = 1;
  MeasuredResistance = ADC_getConvertedValue(ADC_HC15C_Struct);
       I2 = MeasuredResistance / RD;
       Vb = I2 * (RC + RD);
       I1 = (Vb + (RA * I2)) / RA;
       // THIS IS MEASURED RESISTANCE 
       MeasuredResistance = (ADC_REFERENCE - Vb) / I1;
  sprintf(LineText, "Rx: %2.3fV", MeasuredResistance);
  DIP204_clearLine(1);
  DIP204_txt_engine(LineText,1,0,strlen(LineText));

/*  ADC_HC15C_Struct.ADC_Type = ADC_MET_VOLTAGE;
  ADC_HC15C_Struct.ADC_FrontEndDivider = ADC_10V_DIVIDER;
  MeasuredValue = ADC_getConvertedValue(ADC_HC15C_Struct);
  sprintf(LineText, "VMeter: %2.3fV", MeasuredValue);
  DIP204_clearLine(2);
  DIP204_txt_engine(LineText,2,0,strlen(LineText)); */
  //} 
  
  
/*
  // PLAY WITH CAT24C02 MEMORY
  // WRITES THE ARRRAY WriteData - On use WriteData may need to be dynamic allocated
  uint8_t WriteData[] = 
   {
    0x00, // BASE ADDRESS LOCATION WHERE DATA (REST OF ARRAY) WILL BE WRITTEN
    0xab,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0xaa
    }; 
  
  uint8_t WriteData2[] = 
   {
    0x28, // BASE ADDRESS LOCATION WHERE DATA (REST OF ARRAY) WILL BE WRITTEN
    0x56,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x49
    }; 
  
  // Sets up arrray to be read - size is the number of bytes you want to read
  uint8_t ReadData[16];
  uint8_t ViewData[255];
  // init the I2C1
  init_I2C1(I2C_24C0X_FREQUENCY);
  // Write the array
  if (CAT24C0X_write(WriteData, sizeof(WriteData), WriteData[0], SLAVE_ADDRESS_24C0X) == TRUE)
    {
    WriteData[0]=16;
    delayXms(5);
    CAT24C0X_write(WriteData, sizeof(WriteData), WriteData[0], SLAVE_ADDRESS_24C0X);
    delayXms(5);
    delayXms(1);
    for (uint8_t i =0; i<255; i++)
      {
     CAT24C0X_read(ReadData, 1, i, SLAVE_ADDRESS_24C0X);
      ViewData[i]=ReadData[0];
      }
    }

*/  
  
  // PLAY WITH STMPE24M31
  init_WakeFromSleep_EINT();

  /***** FIX ME: WHAT IS PASSED NEEDS TO LOOK LIKE ISR_TimerCounter0 *****/
  //ctl_STMPE24M31_EINT(ISR_STMPE24M31_AandB);
  init_I2C0(I2C_STMPE24M31_FREQUENCY);
  // RESET DEVICE A AND B
  STMPE24M31_reset(STMPE24M31_ACAP);
  STMPE24M31_reset(STMPE24M31_BCAP);
  // INIT A DEVICE AND B
  while (STMPE24M31_init(STMPE24M31_ACAP) != TRUE);
  delayXms(10); // ALLOW START UP IRQ TO CLEAR OR THERE WILL BE BUS CONTENTION IF YOU TRY TO ACCESS A SECOND
  while (STMPE24M31_init(STMPE24M31_BCAP) != TRUE);
  ctl_STMPE24M31_EINT(ISR_STMPE24M31_AandB);
  while(1)
    {
    CLKPWR_Sleep();
    if (KeyPressed_B == TRUE)
      {
      DIP204_clearLine(4);
      switch(BCAP_TouchKey)
        {
        case ((uint32_t)(1<<0)):
        sprintf(LineText,"CS34");
        break;
        
        case ((uint32_t)(1<<1)):
        sprintf(LineText,"CS35");
        break;
        
        case ((uint32_t)(1<<2)):
        sprintf(LineText,"CS47");
        break;
        
        case ((uint32_t)(1<<3)):
        sprintf(LineText,"CS44");
        CLKPWR_DeepSleep();
        break;
        
        case ((uint32_t)(1<<4)):
        sprintf(LineText,"CS32");
        break;
        
        case ((uint32_t)(1<<5)):
        sprintf(LineText,"ENTER");
        break;
        
        case ((uint32_t)(1<<6)):
        sprintf(LineText,"CS19");
        break;
        
        case ((uint32_t)(1<<7)):
        sprintf(LineText,"CS20");
        break;
        
        case ((uint32_t)(1<<8)):
        sprintf(LineText,"CS31");
        break;
        
        case ((uint32_t)(1<<9)):
        sprintf(LineText,"CS36");
        break;
        
        case ((uint32_t)(1<<10)):
        sprintf(LineText,"CS48");
        break;

        case ((uint32_t)(1<<11)):
        sprintf(LineText,"CS9");
        break;
        
        case ((uint32_t)(1<<12)):
        sprintf(LineText,"CS10");
        break;
        
        case ((uint32_t)(1<<13)):
        sprintf(LineText,"CS11");
        break;
        
        case ((uint32_t)(1<<14)):
        sprintf(LineText,"CS23");
        break;
        
        case ((uint32_t)(1<<15)):
        sprintf(LineText,"CS33");
        break;
        
        case ((uint32_t)(1<<16)):
        sprintf(LineText,"CS46");
        break;
        
        case ((uint32_t)(1<<17)):
        sprintf(LineText,"CS45");
        break;
        
        case ((uint32_t)(1<<18)):
        sprintf(LineText,"CS7");
        break;
        
        case ((uint32_t)(1<<19)):
        sprintf(LineText,"CS8");
        break;
        
        case ((uint32_t)(1<<20)):
        sprintf(LineText,"CS12");
        break;
        
        case ((uint32_t)(1<<21)):
        sprintf(LineText,"CS24");
        break;
        
        case ((uint32_t)(1<<22)):
        sprintf(LineText,"CS21");
        break;
        
        case ((uint32_t)(1<<23)):
        sprintf(LineText,"CS22");
        break;
        
        default:
        break;
        }
      DIP204_txt_engine(LineText,4,0,strlen(LineText));
      KeyPressed_B = FALSE;
      }
        
    if (KeyPressed_A == TRUE)
      {
      DIP204_clearLine(4);
      switch(ACAP_TouchKey)
        {
        case ((uint32_t)(1<<0)):
        sprintf(LineText,"CS37");
        break;
        
        case ((uint32_t)(1<<1)):
        sprintf(LineText,"CS27");
        break;
        
        case ((uint32_t)(1<<2)):
        sprintf(LineText,"CS26");
        break;
        
        case ((uint32_t)(1<<3)):
        sprintf(LineText,"CS14");
        break;
        
        case ((uint32_t)(1<<4)):
        sprintf(LineText,"CS15");
        break;
        
        case ((uint32_t)(1<<5)):
        sprintf(LineText,"CS1");
        break;
        
        case ((uint32_t)(1<<6)):
        sprintf(LineText,"CS2");
        break;
        
        case ((uint32_t)(1<<7)):
        sprintf(LineText,"CS3");
        break;
        
        case ((uint32_t)(1<<8)):
        sprintf(LineText,"CS17");
        break;
        
        case ((uint32_t)(1<<9)):
        sprintf(LineText,"CS18");
        break;
        
        case ((uint32_t)(1<<10)):
        sprintf(LineText,"CS30");
        break;

        case ((uint32_t)(1<<11)):
        sprintf(LineText,"CS29");
        break;
        
        case ((uint32_t)(1<<12)):
        sprintf(LineText,"CS28");
        break;
        
        case ((uint32_t)(1<<13)):
        sprintf(LineText,"CS42");
        break;
        
        case ((uint32_t)(1<<14)):
        sprintf(LineText,"CS39");
        break;
        
        case ((uint32_t)(1<<15)):
        sprintf(LineText,"CS38");
        break;
        
        case ((uint32_t)(1<<16)):
        sprintf(LineText,"CS25");
        break;
        
        case ((uint32_t)(1<<17)):
        sprintf(LineText,"CS13");
        break;
        
        case ((uint32_t)(1<<18)):
        sprintf(LineText,"CS4");
        break;
        
        case ((uint32_t)(1<<19)):
        sprintf(LineText,"CS5");
        break;
        
        case ((uint32_t)(1<<20)):
        sprintf(LineText,"CS6");
        break;
        
        case ((uint32_t)(1<<21)):
        sprintf(LineText,"CS16");
        break;
        
        case ((uint32_t)(1<<22)):
        sprintf(LineText,"CS41");
        break;
        
        case ((uint32_t)(1<<23)):
        sprintf(LineText,"CS40");
        break;
        
        default:
        break;
        }
      DIP204_txt_engine(LineText,4,0,strlen(LineText));
      KeyPressed_A = FALSE;
      }
    }
  
  
  // PLAY CP2101
  init_CP2101_polling();
  while(1)
    {
    CP2101_testprint();
    delayXms(500);
    }

}
 // END OF MAIN

/*************************************************************************
 * Function Name: init_WakeFromSleep_EINT
 * Parameters:    void
 * Return:        void
 *
 * Description: Sets up the External IRQ (EINT0) for use as a wake from sleep, deep 
 * sleep or power down.  This function configures the pin settings for the IRQ and enables
 * the IRQ.
 * STEP 1: Define pin assignments for External IRQ - EINT0
 * STEP 2: Set EINT0 as: edge, active low
 * STEP 3: Set priority and enable
 *************************************************************************/
 void init_WakeFromSleep_EINT(void)
 {
  
  // TYPEDEF
  PINSEL_CFG_Type PinCfgStruct;
  EXTI_InitTypeDef EXTICfgstruct;
  
  // STEP 1
  // FROM TABLE 8.5 USER MANUAL LPC17XX
  PinCfgStruct.OpenDrain = 0;
  PinCfgStruct.Pinmode = 0;
  PinCfgStruct.Funcnum = 1;
  PinCfgStruct.Pinnum = 10;
  PinCfgStruct.Portnum = PORT2;
  PINSEL_ConfigPin(&PinCfgStruct);
  
  // STEP 2
  EXTICfgstruct.EXTI_Line = EXTI_EINT0;
  EXTICfgstruct.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
  EXTICfgstruct.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
  EXTI_Config(&EXTICfgstruct);
  
  // STEP 3
  ctl_set_priority(EINT0_IRQn, EINT0_IRQ_PRIORITY);
  ctl_unmask_isr(EINT0_IRQn);
   
 } // END OF FUNCTION init_WakeFromSleep_EINT



/*************************************************************************
 * Function Name: EINT0_IRQHandler
 * Parameters:    void
 * Return:        void
 *
 * Description: IRQ Handler for EINT0.  The purpose of this IRQ is to wake the
 * processor from sleeping.
 * STEP 1: Clear the IRQ condition
 * STEP 2: Perform actions to wake from sleep
 *************************************************************************/
 void EINT0_IRQHandler(void)
 {
 
  // STEP 1
  EXTI_ClearEXTIFlag(EXTI_EINT0);
  ctl_mask_isr(EINT0_IRQn);
  
   
 } // END OF FUNCTION EINT0_IRQHandler
