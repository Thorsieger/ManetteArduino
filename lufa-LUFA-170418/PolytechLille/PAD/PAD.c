/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  OBinou (obconseil [at] gmail [dot] com)
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the RelayBoard program. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "PAD.h"


uint8_t Boutons=0,JoyA=0,JoyB=0;

/** Sends the next HID report to the host, via the keyboard data endpoint. */
void SendNextReport(void)
{//Ne fonctionne pas
if (USB_DeviceState != DEVICE_STATE_Configured)return;

if(Serial_IsCharReceived())
{
	while(!Serial_IsCharReceived());
	Boutons = Serial_ReceiveByte();
	while(!Serial_IsCharReceived());
	JoyA = Serial_ReceiveByte();
	while(!Serial_IsCharReceived());
	JoyB = Serial_ReceiveByte();
}
	
/* Select the PAD Report Endpoint */
Endpoint_SelectEndpoint(PAD_IN_EPADDR1);

/* Check if PAD Endpoint Ready for Read/Write */
if (Endpoint_IsReadWriteAllowed())
{
	/* Write Keyboard Report Data */
	Endpoint_Write_8(Boutons);

	/* Finalize the stream transfer to send the last packet */
	Endpoint_ClearIN();
}

/* Select the PAD Report Endpoint */
Endpoint_SelectEndpoint(PAD_IN_EPADDR2);

/* Check if PAD Endpoint Ready for Read/Write */
if (Endpoint_IsReadWriteAllowed())
{
	/* Write Keyboard Report Data */
	Endpoint_Write_8(JoyA);
	Endpoint_Write_8(JoyB);

	/* Finalize the stream transfer to send the last packet */
	Endpoint_ClearIN();
}
}

/** Reads the next LED status report from the host from the LED data endpoint, if one has been sent. */
void ReceiveNextReport(void)
{
if (USB_DeviceState != DEVICE_STATE_Configured)return;

/* Select the Keyboard LED Report Endpoint */
Endpoint_SelectEndpoint(PAD_OUT_EPADDR);

/* Check if Keyboard LED Endpoint contains a packet */
if (Endpoint_IsOUTReceived())
{
	/* Check to see if the packet contains data */
	if (Endpoint_IsReadWriteAllowed())
	{
		/* Read in the LED report from the host */
		uint8_t LEDReport = Endpoint_Read_8();
		Serial_SendByte(LEDReport);			
	}

	/* Handshake the OUT Endpoint - clear endpoint and ready for next report */
	Endpoint_ClearOUT();
}
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the keyboard device endpoints.
 */
#define PAD_EPSIZE 1
void EVENT_USB_Device_ConfigurationChanged(void)
{
bool ConfigSuccess = true;

/* Setup HID Report Endpoints */
ConfigSuccess &= Endpoint_ConfigureEndpoint(PAD_IN_EPADDR1, EP_TYPE_INTERRUPT, PAD_EPSIZE, 1);
ConfigSuccess &= Endpoint_ConfigureEndpoint(PAD_IN_EPADDR2, EP_TYPE_INTERRUPT, PAD_EPSIZE, 1);
ConfigSuccess &= Endpoint_ConfigureEndpoint(PAD_OUT_EPADDR, EP_TYPE_INTERRUPT, PAD_EPSIZE, 1);
}

/** Configures the board hardware and chip peripherals for the project's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif

	/* Hardware Initialization */
	USB_Init();
	Serial_Init(9600,false);
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
SetupHardware();
GlobalInterruptEnable();

for (;;){
  USB_USBTask();
  //PAD TASK
  ReceiveNextReport();
  SendNextReport();
}
}