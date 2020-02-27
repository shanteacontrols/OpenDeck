/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <avr/power.h>
#include <avr/eeprom.h>
#include <util/crc16.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/constants/Reboot.h"
#include "board/common/io/Helpers.h"
#include "midi/src/Constants.h"
#include "core/src/general/ADC.h"
#include "core/src/arch/avr/Misc.h"
#include "core/src/general/IO.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Reset.h"
#include "board/common/constants/Bootloader.h"
#include "bootloader/mcu/Config.h"

#ifdef FW_BOOT
#include "board/common/usb/descriptors/hid/Redef.h"
#endif

extern "C" void __cxa_pure_virtual()
{
    while (1)
        ;
}

namespace Board
{
    void init()
    {
        DISABLE_INTERRUPTS();

        //clear reset source
        MCUSR &= ~(1 << EXTRF);

        //disable watchdog
        MCUSR &= ~(1 << WDRF);
        wdt_disable();

        //disable clock division
        clock_prescale_set(clock_div_1);

        detail::setup::io();

#ifdef FW_APP
#if !defined(OD_BOARD_16U2) && !defined(OD_BOARD_8U2)
        detail::setup::adc();
#else
        UART::init(UART_USB_LINK_CHANNEL, UART_BAUDRATE_MIDI_OD);
#endif

#ifdef USB_MIDI_SUPPORTED
        detail::setup::usb();
#endif

        detail::setup::timers();
#else
        detail::setup::bootloader();

        //relocate the interrupt vector table to the bootloader section
        //note: if this point is reached, bootloader is active
        MCUCR                = (1 << IVCE);
        MCUCR                = (1 << IVSEL);
#endif

        ENABLE_INTERRUPTS();
    }

    bool checkNewRevision()
    {
        uint16_t crc_eeprom = eeprom_read_word(reinterpret_cast<uint16_t*>(SW_CRC_LOCATION_EEPROM));
#if (FLASHEND > 0xFFFF)
        uint32_t lastAddress = pgm_read_dword_far(core::misc::pgmGetFarAddress(APP_LENGTH_LOCATION));
        uint16_t crc_flash   = pgm_read_word_far(core::misc::pgmGetFarAddress(lastAddress));
#else
        uint32_t lastAddress = pgm_read_dword(APP_LENGTH_LOCATION);
        uint16_t crc_flash   = pgm_read_word(lastAddress);
#endif

        if (crc_eeprom != crc_flash)
        {
            eeprom_update_word(reinterpret_cast<uint16_t*>(SW_CRC_LOCATION_EEPROM), crc_flash);
            return true;
        }

        return false;
    }

    namespace eeprom
    {
        bool read(uint32_t address, LESSDB::sectionParameterType_t type, int32_t& value)
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
                value = eeprom_read_byte(reinterpret_cast<uint8_t*>(address));
                break;

            case LESSDB::sectionParameterType_t::word:
                value = eeprom_read_word(reinterpret_cast<uint16_t*>(address));
                break;

            default:
                // case LESSDB::sectionParameterType_t::dword:
                value = eeprom_read_dword(reinterpret_cast<uint32_t*>(address));
                break;
            }

            return true;
        }

        bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type)
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
                eeprom_update_byte(reinterpret_cast<uint8_t*>(address), value);
                break;

            case LESSDB::sectionParameterType_t::word:
                eeprom_update_word(reinterpret_cast<uint16_t*>(address), value);
                break;

            default:
                // case LESSDB::sectionParameterType_t::dword:
                eeprom_update_dword(reinterpret_cast<uint32_t*>(address), value);
                break;
            }

            return true;
        }
    }    // namespace eeprom

#ifdef FW_BOOT
    namespace bootloader
    {
        void checkPackets()
        {
#ifndef USB_MIDI_SUPPORTED
            static bool    btldrInProgress;
            static uint8_t byteCount;
            static uint8_t lower;
            static uint8_t upper;
            uint8_t        data;

            if (Board::UART::read(UART_USB_LINK_CHANNEL, data))
            {
                if (!btldrInProgress)
                {
                    if (data == COMMAND_START_FLASHING_UART)
                        btldrInProgress = true;
                }
                else
                {
                    if (!byteCount)
                    {
                        lower = data;
                        byteCount++;
                    }
                    else
                    {
                        byteCount = 0;
                        upper     = data;

                        uint16_t word = GET_WORD(lower, upper);
                        packetHandler(word);
                    }
                }
            }
#else
            //nothing, lufa will fire interrupt once packet from usb is received
#endif
        }

        void erasePage(uint32_t address)
        {
            //don't do anything on USB link MCU
#if !defined(OD_BOARD_16U2) && !defined(OD_BOARD_8U2)
            boot_page_erase(address);
            boot_spm_busy_wait();
#endif
        }

        void fillPage(uint32_t address, uint16_t data)
        {
#if !defined(OD_BOARD_16U2) && !defined(OD_BOARD_8U2)
            boot_page_fill(address, data);
#else
            //mark the start of bootloader packets
            //used to avoid possible junk coming on main mcu uart as first byte
            if (!address)
                Board::UART::write(UART_USB_LINK_CHANNEL, COMMAND_START_FLASHING_UART);

            //on USB link MCU, forward the received flash via UART to main MCU
            //send address only when necessary (address first, SPM_PAGESIZE bytes next)

            if (!(address % SPM_PAGESIZE))
            {
                uint16_t lowAddress  = address & 0xFFFF;
                uint16_t highAddress = address >> 16;

                Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(lowAddress));
                Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(lowAddress));

                Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(highAddress));
                Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(highAddress));
            }

            Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(data));
            Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(data));
#endif
        }

        void writePage(uint32_t address)
        {
            //don't do anything on USB link MCU
#if !defined(OD_BOARD_16U2) && !defined(OD_BOARD_8U2)
            //write the filled FLASH page to memory
            boot_page_write(address);
            boot_spm_busy_wait();

            //re-enable RWW section
            boot_rww_enable();
#endif
        }

        void applyFw()
        {
#if defined(OD_BOARD_16U2) || defined(OD_BOARD_8U2)
            //make sure USB link sends the "start application" instruction to the main MCU

            uint16_t lowAddress  = COMMAND_STARTAPPLICATION & 0xFFFF;
            uint16_t highAddress = COMMAND_STARTAPPLICATION >> 16;

            Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(lowAddress));
            Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(lowAddress));

            Board::UART::write(UART_USB_LINK_CHANNEL, LSB_WORD(highAddress));
            Board::UART::write(UART_USB_LINK_CHANNEL, MSB_WORD(highAddress));

            while (!Board::UART::isTxEmpty(UART_USB_LINK_CHANNEL))
                ;
#endif

            core::reset::mcuReset();
        }
    }    // namespace bootloader
#endif

    namespace detail
    {
        void runApplication()
        {
            __asm__ __volatile__(
                // Jump to RST vector
                "clr r30\n"
                "clr r31\n"
                "ijmp\n");
        }

        bool appCRCvalid()
        {
            if (pgm_read_word(0) == 0xFFFF)
                return false;

            return true;
            uint16_t crc = 0x0000;

#if (FLASHEND > 0xFFFF)
            uint32_t lastAddress = pgm_read_word(core::misc::pgmGetFarAddress(APP_LENGTH_LOCATION));
#else
            uint32_t lastAddress = pgm_read_word(APP_LENGTH_LOCATION);
#endif

            for (uint32_t i = 0; i < lastAddress; i++)
            {
#if (FLASHEND > 0xFFFF)
                crc = _crc_xmodem_update(crc, pgm_read_byte_far(core::misc::pgmGetFarAddress(i)));
#else
                crc = _crc_xmodem_update(crc, pgm_read_byte(i));
#endif
            }

#if (FLASHEND > 0xFFFF)
            return (crc == pgm_read_word_far(core::misc::pgmGetFarAddress(lastAddress)));
#else
            return (crc == pgm_read_word(lastAddress));
#endif
        }

        namespace bootloader
        {
            bool isSWtriggerActive()
            {
                return eeprom_read_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION) == BTLDR_REBOOT_VALUE;
            }

            void enableSWtrigger()
            {
                eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, BTLDR_REBOOT_VALUE);
            }

            void clearSWtrigger()
            {
                eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, APP_REBOOT_VALUE);
            }
        }    // namespace bootloader
    }        // namespace detail
}    // namespace Board

#ifdef FW_APP
#ifdef ADC
///
/// \brief ADC ISR used to read values from multiplexers.
///
ISR(ADC_vect)
{
    Board::detail::isrHandling::adc(ADC);
}
#endif
#endif

///
/// \brief Main interrupt service routine.
/// Used to control I/O on board and to update current run time.
///
ISR(TIMER0_COMPA_vect)
{
    static bool _1ms = true;

    _1ms = !_1ms;

    if (_1ms)
    {
        core::timing::detail::rTime_ms++;

#ifdef FW_APP
#if !defined(OD_BOARD_16U2) && !defined(OD_BOARD_8U2)
#ifdef LEDS_SUPPORTED
        Board::detail::io::checkDigitalOutputs();
#endif
#endif
#ifdef LED_INDICATORS
        Board::detail::io::checkIndicators();
#endif
#endif
    }

#ifdef FW_APP
#if !defined(OD_BOARD_16U2) && !defined(OD_BOARD_8U2)
    Board::detail::io::checkDigitalInputs();
#endif
#endif
}
