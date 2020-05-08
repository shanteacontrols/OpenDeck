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