/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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
 *  Main source file for the Mass Storage class bootloader. This file contains the complete bootloader logic.
 */

#define  INCLUDE_FROM_BOOTLOADER_MASSSTORAGE_C
#include "BootloaderMassStorage.h"
#include "inc/PinManipulation.h"
#include "inc/Pins.h"
#include "inc/SignalLogic.h"

/** LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Device_t Disk_MS_Interface =
	{
		.Config =
			{
				.InterfaceNumber           = INTERFACE_ID_MassStorage,
				.DataINEndpoint            =
					{
						.Address           = MASS_STORAGE_IN_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.DataOUTEndpoint           =
					{
						.Address           = MASS_STORAGE_OUT_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.TotalLUNs                 = 1,
			},
	};

/** Flag to indicate if the bootloader should be running, or should exit and allow the application code to run
 *  via a soft reset. When cleared, the bootloader will abort, the USB interface will shut down and the application
 *  started via a forced watchdog reset.
 */
bool RunBootloader = true;

/** Magic lock for forced application start. If the HWBE fuse is programmed and BOOTRST is unprogrammed, the bootloader
 *  will start if the /HWB line of the AVR is held low and the system is reset. However, if the /HWB line is still held
 *  low when the application attempts to start via a watchdog reset, the bootloader will re-start. If set to the value
 *  \ref MAGIC_BOOT_KEY the special init function \ref Application_Jump_Check() will force the application to start.
 */
uint16_t MagicBootKey ATTR_NO_INIT;

/** Indicates if the bootloader is allowed to exit immediately if \ref RunBootloader is \c false. During shutdown all
 *  pending commands must be processed before jumping to the user-application, thus this tracks the main program loop
 *  iterations since a SCSI command from the host was received.
 */
static uint8_t TicksSinceLastCommand = 0;

#define REBOOT_VALUE_EEPROM_LOCATION    EEPROM_FILE_SIZE_BYTES
#define BTLDR_REBOOT_VALUE              0x47
#define APP_REBOOT_VALUE                0xFF

/** Special startup routine to check if the bootloader was started via a watchdog reset, and if the magic application
 *  start key has been loaded into \ref MagicBootKey. If the bootloader started via the watchdog and the key is valid,
 *  this will force the user application to start via a software jump.
 */
void Application_Jump_Check(void)
{
	bool JumpToApplication = false;

	/* Check if the device's BOOTRST fuse is set */
    //note: it is set on opendeck board
	if (boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS) & FUSE_BOOTRST)
	{
        //if rx/tx MIDI pins are connected together on startup, jump to bootloader
        //configure rx/tx pins
        setInput(RX_TX_PORT, RX_PIN);
        setOutput(RX_TX_PORT, TX_PIN);

        //set tx pin to 0V
        setLow(RX_TX_PORT, TX_PIN);

        //add some delay before reading pin
        _delay_ms(10);

        bool hardwareTrigger = readPin(RX_TX_PORT, RX_PIN) == BOOTLOADER_ENABLE_SIGNAL;
        bool softwareTrigger = eeprom_read_byte((uint8_t*)EEPROM_FILE_SIZE_BYTES) == BTLDR_REBOOT_VALUE;

        if (!hardwareTrigger && !softwareTrigger)
            JumpToApplication = true;

		/* Clear reset source */
		MCUSR &= ~(1 << EXTRF);
	}

	/* Don't run the user application if the reset vector is blank (no app loaded) */
	bool ApplicationValid = (pgm_read_word_near(0) != 0xFFFF);

	/* If a request has been made to jump to the user application, honor it */
	if (JumpToApplication && ApplicationValid)
	{
		/* Turn off the watchdog */
		MCUSR &= ~(1 << WDRF);
		wdt_disable();

		/* Clear the boot key and jump to the user application */
		eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, APP_REBOOT_VALUE);

		// cppcheck-suppress constStatement
		((void (*)(void))0x0000)();
	}
}

/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	while (RunBootloader || TicksSinceLastCommand++ < 0xFF)
	{
		MS_Device_USBTask(&Disk_MS_Interface);
		USB_USBTask();
	}

	/* Disconnect from the host - USB interface will be reset later along with the AVR */
	USB_Detach();

    //opendeck specific - blink bootloader led few times
    for (int i=0; i<2; i++)
    {
        if (i%2)
        {
            setHigh(LED_OUT_PORT, LED_OUT_PIN);
            setHigh(LED_IN_PORT, LED_IN_PIN);
        }
        else
        {
            setLow(LED_OUT_PORT, LED_OUT_PIN);
            setLow(LED_IN_PORT, LED_IN_PIN);
        }
        _delay_ms(250);
    }

    eeprom_update_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, 0xFF);

    setLow(LED_OUT_PORT, LED_OUT_PIN);
    setLow(LED_IN_PORT, LED_IN_PIN);

	/* Enable the watchdog and force a timeout to reset the AVR */
	wdt_enable(WDTO_250MS);

	for (;;);
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
static void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Relocate the interrupt vector table to the bootloader section */
	MCUCR = (1 << IVCE);
	MCUCR = (1 << IVSEL);

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();

    setOutput(LED_OUT_PORT, LED_OUT_PIN);
    setOutput(LED_IN_PORT, LED_IN_PIN);
    setHigh(LED_OUT_PORT, LED_OUT_PIN);
    setHigh(LED_IN_PORT, LED_IN_PIN);
}

/** ISR to periodically toggle the LEDs on the board to indicate that the bootloader is active. */
ISR(TIMER1_OVF_vect, ISR_BLOCK)
{
	LEDs_ToggleLEDs(LEDS_LED1 | LEDS_LED2);
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs. */
void EVENT_USB_Device_Connect(void)
{
	/* Indicate USB enumerating */
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs and stops the Mass Storage management task.
 */
void EVENT_USB_Device_Disconnect(void)
{
	/* Indicate USB not ready */
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup Mass Storage Data Endpoints */
	ConfigSuccess &= MS_Device_ConfigureEndpoints(&Disk_MS_Interface);

	/* Indicate endpoint configuration success or failure */
	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	MS_Device_ProcessControlRequest(&Disk_MS_Interface);
}

/** Mass Storage class driver callback function the reception of SCSI commands from the host, which must be processed.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface configuration structure being referenced
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	bool CommandSuccess;

	LEDs_SetAllLEDs(LEDMASK_USB_BUSY);
	CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);
	LEDs_SetAllLEDs(LEDMASK_USB_READY);

	/* Signal that a command was processed, must not exit bootloader yet */
	TicksSinceLastCommand = 0;

	return CommandSuccess;
}
