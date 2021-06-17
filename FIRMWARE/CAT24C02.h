/*****************************************************************
 *
 * File name:       CAT24C02.H
 * Description:     Project definitions and function prototypes for use with ADC_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/20/2011
 * Hardware:               
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/ 

#ifndef _CAT24C02_DEFINES
#define _CAT24C02_DEFINES


// INCLUDES
#include "HC15C_DEFINES.h"
#include "CMSIS\lpc17xx_pinsel.h"
#include "CMSIS\lpc17xx_i2c.h"


// ENUMERATED TYPES AND STRUCTURES


// STRUCTURES


// DEFINES
#define SLAVE_ADDRESS_24C0X   (0xA0>>1)   // A0 ADDRESS - BUT 7BIT ADDRESS IS USED INSTEAD
#define I2C_RETRY_ATTEMPS     3           // NUMBER OF RE-TRYS BEFORE FAIL
#define I2C_24C0X_FREQUENCY   100000      // VALUE IN Hz
#define MAX_EEPROM_SIZE       128         // VALUE IN BYTES 128 FOR C01, 256 FOR C02


// PROTOTYPE FUNCITONS
void init_I2C1(uint32_t);
uint8_t CAT24C0X_write(uint8_t *, uint8_t, uint8_t, uint8_t);
uint8_t CAT24C0X_read(uint8_t *, uint8_t, uint8_t, uint8_t);

#endif