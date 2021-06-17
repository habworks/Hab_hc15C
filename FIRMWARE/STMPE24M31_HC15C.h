/*****************************************************************
 *
 * File name:       CAT24C02.H
 * Description:     Project definitions and function prototypes for use with ADC_HC15C.c
 * Author:          Hab S. Collector
 * Date:            7/20/2011
 * Hardware:        HC15C      
 * Firmware Tool:   CrossStudio for ARM
 * Notes:           This file should be written as to not be dependent 
 *                  on other includes - everything these functions need should be passed to them
 *
 *****************************************************************/ 

#ifndef _STMPE24M31_DEFINES
#define _STMPE24M31_DEFINES


// INCLUDES
#include <ctl_api.h>
#include "HC15C_DEFINES.h"
#include "CMSIS\lpc17xx_pinsel.h"
#include "CMSIS\lpc17xx_i2c.h"
#include "CMSIS\lpc17xx_exti.h"


// ENUMERATED TYPES AND STRUCTURES
enum STMPE24M31_DEVICES
     {
     STMPE24M31_ACAP,
     STMPE24M31_BCAP
     };

enum REGISTER_AND_VALUE
     {
     REGISTER_BYTE,
     VALUE_BYTE
     };


// STRUCTURES


// DEFINES
// I2C SPECIFIC INFO
#define SLAVE_ADDRESS_STMPE24M31_ACAP (0x58)   // 0X58 ADDRESS - BUT 7BIT ADDRESS IS USED INSTEAD
#define SLAVE_ADDRESS_STMPE24M31_BCAP (0x59)   // 0X59 ADDRESS - BUT 7BIT ADDRESS IS USED INSTEAD
#define I2C_STMPE24M31_RETRY_ATTEMPS  3
#define I2C_STMPE24M31_FREQUENCY      400000      // VALUE IN Hz
// REGISTER SPECIFIC INFO
#define STMPE24M31_SYSCON3_REG       (0x03)
#define STMPE24M31_INT_STA_REG        (0x08)
#define STMPE24M31_KEY_FILT_DATA_REG  (0x9A)
// INT STA (0x08) MASK SPECIFIC INFO
#define INT_STA_TOUCH                 (0x04)
#define INT_STA_END_OF_CALIBRATION    (0x08)
#define INT_STA_ABNORMAL_CONDITION    (0x10)
#define INT_STA_WAKEUP                (0x20)
// SYSCON-1 (0x03) MASK SPECIFIC INFO
#define SYSCON3_SOFT_RESET           (0x02)


// PROTOTYPE FUNCITONS
void init_I2C0(uint32_t);
uint8_t STMPE24M31_write(uint8_t *, uint8_t, uint8_t);
uint8_t STMPE24M31_read(uint8_t *, uint8_t, uint8_t, uint8_t);
uint8_t STMPE24M31_init(uint8_t);
uint8_t STMPE24M31_reset(uint8_t);
// IRQ SPECIFIC
void ctl_STMPE24M31_EINT(CTL_ISR_FN_t);
void EINT1_IRQHandler(void);
void EINT2_IRQHandler(void);
void ISR_STMPE24M31_AandB(uint8_t);


#endif