/*

Copyright 2015-2020 Igor Petrovic

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
#include <avr/wdt.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/constants/Reboot.h"
#include "core/src/general/ADC.h"
#include "core/src/arch/avr/Misc.h"
#include "core/src/general/IO.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Reset.h"

extern "C" void __cxa_pure_virtual()
{
    while (1)
        ;
}

namespace Board
{
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
        namespace setup
        {
            void application()
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

#ifndef USB_LINK_MCU
                detail::setup::adc();
#else
                Board::UART::init(UART_USB_LINK_CHANNEL, UART_BAUDRATE_MIDI_OD);
#endif

#ifdef USB_MIDI_SUPPORTED
                detail::setup::usb();
#endif

                detail::setup::timers();

                ENABLE_INTERRUPTS();
            }

            void bootloader()
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
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board