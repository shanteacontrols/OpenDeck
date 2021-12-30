/*

Copyright 2015-2021 Igor Petrovic

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

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/Helpers.h"
#include "core/src/arch/avr/Misc.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Timing.h"
#include <MCU.h>

/// Location at which reboot type is written in EEPROM when initiating software reset.
#define REBOOT_VALUE_EEPROM_LOCATION EEPROM_END

namespace Board
{
    namespace bootloader
    {
        uint8_t magicBootValue()
        {
            return eeprom_read_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION);
        }

        void setMagicBootValue(uint8_t value)
        {
            eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, static_cast<uint8_t>(value));
        }

        void runBootloader()
        {
#if defined(LED_INDICATORS) && defined(LED_INDICATORS_CTL)
            // the only reason to run this in bootloader is to control led indicators through data event timeouts
            // if this feature is unavailable, don't configure the timer
            detail::setup::timers();
#endif

            detail::setup::usb();
        }

        void runApplication()
        {
            detail::io::ledFlashStartup();

            __asm__ __volatile__(
                // Jump to RST vector
                "clr r30\n"
                "clr r31\n"
                "ijmp\n");
        }
    }    // namespace bootloader
}    // namespace Board