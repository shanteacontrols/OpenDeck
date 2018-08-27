/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
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
 *  Main source file for the HID class bootloader. This file contains the complete bootloader logic.
 */

#include "BootloaderHID.h"
#include "core/src/HAL/avr/PinManipulation.h"
#include "core/src/general/BitManipulation.h"
#include <util/crc16.h>
#include "board/common/constants/LEDs.h"
#include "board/avr/variants/Common.h"
#include "board/common/constants/Reboot.h"
#include "pins/Pins.h"
#include "board/common/indicators/Variables.h"
#include "board/Board.h"

#ifdef CRC_CHECK

///
/// \brief Calculates CRC of entire flash.
/// \return True if CRC is valid, that is, if it matches CRC written in last flash address.
///
bool appCRCvalid()
{
    uint16_t crc = 0x0000;
    int16_t lastAddress = pgm_read_word_near(APP_LENGTH_LOCATION);

    for (int i=0; i<lastAddress; i++)
    {
        crc = _crc_xmodem_update(crc, pgm_read_byte_near(i));
    }

    return (crc == pgm_read_word_near(lastAddress));
}

#endif

///
/// Flag to indicate if the bootloader should be running, or should exit and allow the application code to run
/// via a soft reset. When cleared, the bootloader will abort, the USB interface will shut down and the application
/// started via a forced watchdog reset.
///
static bool RunBootloader = true;

volatile bool UARTreceived, UARTsent;

///
/// \brief Checks if application should be run.
/// This function performs two checks: hardware and software bootloader entry.
/// Hardware bootloader entry is possible if the specific board has defined button
/// which should be pressed before the MCU is turned on. If it is, bootloader is
/// entered.
/// Software bootloader entry is possible by writing special value to special EEPROM
/// address before the application is rebooted.
/// \returns True if application should be run, false if bootloader should be run.
///
bool checkApplicationRun()
{
    bool jumpToApplication = false;

    #if defined(BOARD_KODAMA) || defined(BOARD_TANNIN)
    //these boards use input shift registers
    //read 8 inputs and then read only specific input used as an
    //hardware bootloader entry button
    uint16_t dInData = 0;
    setLow(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
    setLow(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);
    _NOP();
    _NOP();

    setHigh(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

    #ifdef BOARD_KODAMA
    //this board has two shift registers - 16 inputs in total
    for (int i=0; i<16; i++)
    #elif defined(BOARD_TANNIN)
    //single register
    for (int i=0; i<8; i++)
    #endif
    {
        setLow(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
        _NOP();
        BIT_WRITE(dInData, i, !readPin(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN));
        setHigh(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
    }

    bool hardwareTrigger = BIT_READ(dInData, BTLDR_BUTTON_INDEX);
    #else
    #ifdef BTLDR_BUTTON_PORT
    bool hardwareTrigger = !readPin(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
    #else
    //no hardware entry possible in this case
    bool hardwareTrigger = false;
    #endif
    #endif

    //check if user wants to enter bootloader
    bool softwareTrigger = eeprom_read_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION) == BTLDR_REBOOT_VALUE;

    //reset value in eeprom after reading
    eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, APP_REBOOT_VALUE);

    //jump to app only if both software and hardware triggers aren't activated
    if (!hardwareTrigger && !softwareTrigger)
        jumpToApplication = true;

    //don't run the user application if the reset vector is blank (no app loaded)
    bool applicationValid = (pgm_read_word_near(0) != 0xFFFF);

    return (jumpToApplication && applicationValid);
}

///
/// \brief Main program entry point.
/// This routine configures the hardware required by the bootloader, then continuously
/// runs the bootloader processing routine until instructed to soft-exit.
///
int main(void)
{
    //clear reset source
    MCUSR &= ~(1 << EXTRF);

    initPins();

    if (checkApplicationRun())
    {
        //disable watchdog
        MCUSR &= ~(1 << WDRF);
        wdt_disable();

        //run app
        ((void (*)(void))0x0000)();
    }

    //setup hardware required for the bootloader
    setupHardware();

    //enable global interrupts so that the USB stack can function
    GlobalInterruptEnable();

    while (RunBootloader)
    {
        USB_USBTask();
    }

    //disconnect from the host - USB interface will be reset later along with the AVR
    USB_Detach();

    //enable the watchdog and force a timeout to reset the AVR
    wdt_enable(WDTO_250MS);

    for (;;);
}

///
/// \brief Configures all hardware required for the bootloader.
///
static void setupHardware(void)
{
    //disable watchdog if enabled by bootloader/fuses
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    //disable clock division
    clock_prescale_set(clock_div_1);

    //relocate the interrupt vector table to the bootloader section
    MCUCR = (1 << IVCE);
    MCUCR = (1 << IVSEL);

    //initialize USB subsystem
    USB_Init();

    #ifdef BOARD_A_xu2
    Board::initUART(38400, 0);
    #endif
}

///
/// \brief Initializes all pins needed in bootloader mode.
///
static void initPins()
{
    //indicate that we're in bootloader mode by turning on specific leds
    //if the board uses indicator leds for midi traffic, turn them both on
    #ifdef BOARD_KODAMA
    setLow(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

    for (int i=0; i<16; i++)
    {
        EXT_LED_ON(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
        pulseHighToLow(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
    }

    setHigh(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
    #elif defined (LED_INDICATORS)
    setOutput(LED_IN_PORT, LED_IN_PIN);
    setOutput(LED_OUT_PORT, LED_OUT_PIN);
    INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
    INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
    #endif

    //configure bootloader entry pins
    #if defined(BOARD_TANNIN) || defined(BOARD_KODAMA)
    setInput(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN);
    setOutput(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
    setOutput(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);
    #ifdef BOARD_TANNIN
    //select column 6 in button matrix
    setOutput(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    setOutput(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    setOutput(DEC_DM_A2_PORT, DEC_DM_A2_PIN);
    setHigh(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    setLow(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    setHigh(DEC_DM_A2_PORT, DEC_DM_A2_PIN);
    #endif
    #else
    #ifdef BTLDR_BUTTON_PORT
    setInput(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
    setHigh(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
    #endif
    #endif
}

///
/// \brief Event handler for the USB_ConfigurationChanged event.
/// This configures the device's endpoints ready to relay data to and from the attached USB host.
///
void EVENT_USB_Device_ConfigurationChanged(void)
{
    //setup HID Report Endpoint
    Endpoint_ConfigureEndpoint(HID_IN_EPADDR, EP_TYPE_INTERRUPT, HID_IN_EPSIZE, 1);
}

///
/// \brief Event handler for the USB_ControlRequest event.
/// This is used to catch and process control requests sent to the device
/// from the USB host before passing along unhandled control requests to the
/// library for processing internally.
///
void EVENT_USB_Device_ControlRequest(void)
{
    //ignore any requests that aren't directed to the HID interface
    if ((USB_ControlRequest.bmRequestType & (CONTROL_REQTYPE_TYPE | CONTROL_REQTYPE_RECIPIENT)) !=
        (REQTYPE_CLASS | REQREC_INTERFACE))
    {
        return;
    }

    //process HID specific control requests
    switch (USB_ControlRequest.bRequest)
    {
        case HID_REQ_SetReport:
        Endpoint_ClearSETUP();

        //wait until the command has been sent by the host
        while (!(Endpoint_IsOUTReceived()));

        //read in the write destination address
        #if (FLASHEND > 0xFFFF)
        uint32_t PageAddress = ((uint32_t)Endpoint_Read_16_LE() << 8);
        #else
        uint16_t PageAddress = Endpoint_Read_16_LE();
        #endif

        //check if the command is a program page command, or a start application command
        #if (FLASHEND > 0xFFFF)
        if ((uint16_t)(PageAddress >> 8) == COMMAND_STARTAPPLICATION)
        #else
        if (PageAddress == COMMAND_STARTAPPLICATION)
        #endif
        {
            #ifdef CRC_CHECK
            if (!appCRCvalid())
            {
                while (1)
                {
                    //indicate error by flashing indicator leds
                    #ifdef LED_INDICATORS
                    INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
                    INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
                    _delay_ms(500);
                    INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
                    INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
                    _delay_ms(500);
                    #endif
                }
            }
            #endif

            RunBootloader = false;
        }
        else if (PageAddress < BOOT_START_ADDR)
        {
            //erase the given FLASH page, ready to be programmed
            boot_page_erase(PageAddress);
            boot_spm_busy_wait();

            //write each of the FLASH page's bytes in sequence
            for (uint8_t PageWord = 0; PageWord < (SPM_PAGESIZE / 2); PageWord++)
            {
                //check if endpoint is empty - if so clear it and wait until ready for next packet
                if (!(Endpoint_BytesInEndpoint()))
                {
                    Endpoint_ClearOUT();
                    while (!(Endpoint_IsOUTReceived()));
                }

                //write the next data word to the FLASH page
                boot_page_fill(PageAddress + ((uint16_t)PageWord << 1), Endpoint_Read_16_LE());
            }

            //write the filled FLASH page to memory
            boot_page_write(PageAddress);
            boot_spm_busy_wait();

            //re-enable RWW section
            boot_rww_enable();
        }

        Endpoint_ClearOUT();

        Endpoint_ClearStatusStage();
        break;
    }
}