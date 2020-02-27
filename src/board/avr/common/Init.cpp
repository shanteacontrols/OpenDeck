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
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/constants/Reboot.h"
#include "core/src/general/ADC.h"
#include "core/src/arch/avr/Misc.h"
#include "core/src/general/IO.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Reset.h"
#include "bootloader/mcu/Config.h"

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
#ifndef USB_LINK_MCU
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

        bool isAppCRCvalid()
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
    }    // namespace detail
}    // namespace Board