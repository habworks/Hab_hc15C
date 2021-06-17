/*****************************************************************
 *
 * File name:       ADC_HC15C.H
 * Description:     Project definitions and function prototypes for use with ADC_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/12/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/
 
 // INCLUDES
#include "ADC_HC15C.H"

// GLOBAL VARS

// EXTERNS
extern void delayXms(uint8_t);




/*************************************************************************
 * Function Name: init_ADC_polling
 * Parameters: void
 * Return: void
 *
 * Description: Configures P0.23 for polling
 * can be called.
 * STEP 1: Configure P0.23 and P0.24 as ADC pins
 * STEP 2: Configure ADC Meter range and control pins: set default conditions 
 * STEP 3: Configure PWM in timer mode
 *************************************************************************/
 void init_ADC_polling(void)
  {
 /////////////////////////FIX ME ADD OPTION FOR OTHER CHANNELS
 
  PINSEL_CFG_Type PINSEL_PinCfgStruct;
 
  // STEP 1
  // FROM TABLE 8.5 USER MANUAL LPC17XX - NOTE: P0.23 IS ADC_BAT
  PINSEL_PinCfgStruct.Funcnum = 1;
  PINSEL_PinCfgStruct.OpenDrain = 0;
  PINSEL_PinCfgStruct.Pinmode = 0;
  PINSEL_PinCfgStruct.Pinnum = 23;
  PINSEL_PinCfgStruct.Portnum = PORT0;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  // P0.24 IS ADC_MET
  PINSEL_PinCfgStruct.Pinnum = 24;
  PINSEL_ConfigPin(&PINSEL_PinCfgStruct);
  
  // STEP 2
  GPIO_SetDir(PORT1,(RANGE_10V|RANGE_20V|RANGE_30V|SEL_V_C),1);
  // SET DEFAULT FOR 30V RANGE ON VOLTAGE MEASURE
  GPIO_SetValue(PORT1, (RANGE_30V|SEL_V_C));
  
  // STEP 3
  ADC_Init(LPC_ADC, 100000);
  ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,DISABLE);
  ADC_IntConfig(LPC_ADC,ADC_ADINTEN1,DISABLE);
  ADC_ChannelCmd(LPC_ADC,0,DISABLE);
  ADC_ChannelCmd(LPC_ADC,1,DISABLE);
  
  }




/*************************************************************************
 * Function Name: ADC_getConvertedValue
 * Parameters: ADC_HC15C_Type
 * Return: float
 *
 * Description: Retrieves the conversion for the specified ADC Channel
 * can be called.  
 * STEP 1: Issue a conversion command according to ADC channel
 * STEP 2: Get the Count by polling until done
 * STEP 3: Convert the count to the voltage reading
 * NOTE: This a a polling routine.  Use with init_ADC_polling(void) called first
 *************************************************************************/
 double ADC_getConvertedValue(ADC_HC15C_Type ADC_HC15C_Struct)
 {
 
 uint8_t Count;
 uint32_t ADC_ConvertValue = 0, ADC_Conversion = 0;
 double ADC_Voltage;
 
 // STEP 1
 Count = ADC_HC15C_Struct.ADC_AvgWeight;
 
 switch (ADC_HC15C_Struct.ADC_Type)
   {
   case 0:
   ADC_ChannelCmd(LPC_ADC,0,ENABLE);
   ADC_ChannelCmd(LPC_ADC,1,DISABLE);
   ADC_ChannelCmd(LPC_ADC,2,DISABLE);
   break;
   
   case 1:
   ADC_ChannelCmd(LPC_ADC,0,DISABLE);
   ADC_ChannelCmd(LPC_ADC,1,ENABLE);
   ADC_ChannelCmd(LPC_ADC,2,DISABLE);
   break;
   }

 // CLEAR ANY DONE OR OVERRUN BIT BY A DUMMY READ OF THE REGISTERS: GLOBAL AND SPECIFIC
 //delayXms(10);
 //ADC_Conversion = ADC_ChannelGetData(LPC_ADC, ADC_HC15C_Struct.ADC_Type);
 //ADC_Conversion = ADC_GlobalGetData(LPC_ADC);
 //ADC_Conversion = ADC_ChannelGetData(LPC_ADC, ADC_HC15C_Struct.ADC_Type);

 // STEP 2
 
 do
   {
   ADC_StartCmd(LPC_ADC, ADC_START_NOW);
   while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_HC15C_Struct.ADC_Type, ADC_DATA_DONE)));
   //while(!(ADC_GlobalGetStatus(LPC_ADC, ADC_DATA_DONE)));
	 ADC_Conversion = ADC_ChannelGetData(LPC_ADC, ADC_HC15C_Struct.ADC_Type);
   ADC_ConvertValue += ADC_Conversion;
   Count--;
   delayXms(3);
   } while (Count > 0);
   
 // STEP 3
 ADC_Voltage = ((double)ADC_ConvertValue/ADC_HC15C_Struct.ADC_AvgWeight);
 ADC_Voltage = (ADC_Voltage/ADC_FULL_COUNT) * (1.0/ADC_HC15C_Struct.ADC_FrontEndDivider) * ADC_REFERENCE;
 //ADC_Voltage -= 0.005;
 //ADC_Voltage = ADC_Voltage * 1.005;
 
 return(ADC_Voltage);
 
 } // END OF FUNCTION ADC_getConvertedValue
   
 