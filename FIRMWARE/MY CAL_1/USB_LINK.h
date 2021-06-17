/*****************************************************************
 *
 * File name:         USB_LINK.H
 * Description:       Project definitions and function prototypes for use with USB_LINK.c
 * Author:            Hab S. Collector
 * Date:              12/31/2011
 * Hardware:               
 * Firmware Tool:     CrossStudio for ARM
 * Notes:             This file should be written as to not be dependent 
 *                    on other includes - everything these functions need should be passed to them
*****************************************************************/ 

#ifndef _USB_LINK_DEFINES
#define _USB_LINK_DEFINES

// INCLUDES
#include "HC15C_DEFINES.h"



// PROTOTYPES
void call_USB_VCOM_Link(void);
void call_WriteToUSB_Meter(double, uint8_t);
void call_USB_VCOM_UnLink(void);

#endif