/*****************************************************************
 *
 * File name:       ADC_HC15C.H
 * Description:     Project definitions and function prototypes for use with ADC_HC15C.c
 * Author:          Hab S. Collector
 * Date:            11/12/2011
 * LAST EDIT:       7/21/2012 
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/
 
 // INCLUDES
#include "ADC_HC15C.H"
#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include <ctl_api.h>

// GLOBAL VARS


// EXTERNS
extern void delayXms(uint32_t);
extern CTL_MUTEX_t ADC_Mutex;




/*************************************************************************
 * Function Name: init_ADC_polling
 * Parameters: void
 * Return: void
 *
 * Description: Configures P0.23 and P0.24 for ADC polling (Meter and Vbat channel)
 * Must be used before conversion commands can be called.
 * STEP 1: Configure P0.23 and P0.24 as ADC pins
 * STEP 2: Configure ADC Meter range and control pins as outputs and set default conditions 
 * STEP 3: Set conversion rate, set for polling (no IRQ), and channels to disable 
 *************************************************************************/
 void init_ADC_polling(void)
  {
 
  PINSEL_CFG_Type PINSEL_PinCfgStruct;
 
  // STEP 1
  // FROM TABLE 8.5 USER MANUAL LPC17XX SET UP AS ADC FUNCTION
  // P0.23 IS ADC_BAT
  PINSEL_PinCfgStruct.Funcnum = 1;
  PINSEL_PinCfgStruct.OpenDrain = 0;
  PINSEL_PinCfgStruct.Pinmode = 0;
  PINSEL_PinCfgStruct.Pinnum = 23;
  PINSEL_PinCfgStruct.Portnum = PORT0;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  // P0.24 IS ADC_MET
  PINSEL_PinCfgStruct.Pinnum = 24;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  // PO.25 IS ADC_VUSB
  PINSEL_PinCfgStruct.Pinnum = 25;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  
  // STEP 2
  GPIO_SetDir(PORT1,(RANGE_10V|RANGE_20V|RANGE_30V|SEL_V_C),1);
  // SET DEFAULT FOR 30V RANGE ON VOLTAGE MEASURE
  GPIO_SetValue(PORT1, (RANGE_30V|SEL_V_C));
  
  // STEP 3
  ADC_Init(LPC_ADC, ADC_CONVERSION_RATE);
  ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,DISABLE);
  ADC_IntConfig(LPC_ADC,ADC_ADINTEN1,DISABLE);
  ADC_IntConfig(LPC_ADC,ADC_ADINTEN2,DISABLE);
  ADC_ChannelCmd(LPC_ADC,ADC_BAT_VOLTAGE,DISABLE);
  ADC_ChannelCmd(LPC_ADC,ADC_MET_VOLTAGE,DISABLE);
  ADC_ChannelCmd(LPC_ADC,ADC_USB_VOLTAGE,DISABLE);
  
  } // END OF init_ADC_polling




/*************************************************************************
 * Function Name: ADC_getConvertedValue
 * Parameters: ADC_HC15C_Type
 * Return: double
 *
 * Description: Retrieves the conversion result for the specified ADC Channel.
 * Returns a result that has been averaged by the structure avg value.  If avg
 * value set to zero - no averaging occurs.  The samples are taken on a periodic interval,
 * hence the total time it takes to do a conversion can be large: (Convert Time + Interval)x Number of Samples
 * NOTE: This a a polling routine.  Use with init_ADC_polling - called first
 * NOTE: Multiple calling tasks - must be thread safe
 * STEP 1: Issue a conversion command according to ADC channel
 * STEP 2: Get the Count by polling until done
 * STEP 3: Convert the count to the voltage reading
 *************************************************************************/
 double ADC_getConvertedValue(ADC_HC15C_Type ADC_HC15C_Struct)
 {
 
 ctl_mutex_lock(&ADC_Mutex, CTL_TIMEOUT_NONE, 0); 
   
 uint8_t Count;
 uint32_t ADC_ConvertValue = 0, ADC_Conversion = 0;
 double ADC_Voltage;
 
 // STEP 1
 Count = ADC_HC15C_Struct.ADC_AvgWeight;
 switch (ADC_HC15C_Struct.ADC_Type)
   {
   case ADC_BAT_VOLTAGE:
   ADC_ChannelCmd(LPC_ADC,ADC_BAT_VOLTAGE,ENABLE);
   ADC_ChannelCmd(LPC_ADC,ADC_MET_VOLTAGE,DISABLE);
   ADC_ChannelCmd(LPC_ADC,ADC_USB_VOLTAGE,DISABLE);
   break;
   
   case ADC_MET_VOLTAGE:
   ADC_ChannelCmd(LPC_ADC,ADC_BAT_VOLTAGE,DISABLE);
   ADC_ChannelCmd(LPC_ADC,ADC_MET_VOLTAGE,ENABLE);
   ADC_ChannelCmd(LPC_ADC,ADC_USB_VOLTAGE,DISABLE);
   break;
   
   case ADC_USB_VOLTAGE:
   ADC_ChannelCmd(LPC_ADC,ADC_BAT_VOLTAGE,DISABLE);
   ADC_ChannelCmd(LPC_ADC,ADC_MET_VOLTAGE,DISABLE);
   ADC_ChannelCmd(LPC_ADC,ADC_USB_VOLTAGE,ENABLE);
   break;
   }
 ADC_Conversion = ADC_GlobalGetData(LPC_ADC);
 ADC_Conversion = ADC_ChannelGetData(LPC_ADC, ADC_HC15C_Struct.ADC_Type);
 
 // STEP 2
 do
   { 
   ADC_StartCmd(LPC_ADC, ADC_START_NOW);
   while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_HC15C_Struct.ADC_Type, ADC_DATA_DONE)));
	 ADC_Conversion = ADC_ChannelGetData(LPC_ADC, ADC_HC15C_Struct.ADC_Type);
   ADC_ConvertValue += ADC_Conversion;
   Count--;
   delayXms(ADC_HC15C_Struct.ADC_msTimeBetweenSamples);
   } while (Count > 0);
   
 // STEP 3
 ADC_Voltage = ((double)ADC_ConvertValue/ADC_HC15C_Struct.ADC_AvgWeight);
 ADC_Voltage = (ADC_Voltage/ADC_FULL_COUNT) * (1.0/ADC_HC15C_Struct.ADC_FrontEndDivider) * ADC_REFERENCE;
 
 ctl_mutex_unlock(&ADC_Mutex);
 return(ADC_Voltage);
 
 } // END OF FUNCTION ADC_getConvertedValue
   
 