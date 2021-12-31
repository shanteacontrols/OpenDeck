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

#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/io/Helpers.h"
#include "core/src/general/ADC.h"
#include "core/src/arch/avr/Misc.h"
#include "core/src/general/IO.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Timing.h"
#include <Pins.h>

// serial numbers are available only on AVR MCUs with USB
#ifdef USB_SUPPORTED
#include "LUFA/Drivers/USB/USB.h"
#endif

namespace Board
{
#ifdef USB_SUPPORTED
    void uniqueID(uniqueID_t& uid)
    {
        ATOMIC_SECTION
        {
            uint8_t address = INTERNAL_SERIAL_START_ADDRESS;

            for (uint8_t i = 0; i < (UID_BITS / 8); i++)
            {
                uid[i] = boot_signature_byte_get(address++);

                // LUFA sends unique ID with nibbles swaped
                // to match with LUFA, invert them here
                uid[i] = (uid[i] << 4) | ((uid[i] >> 4) & 0x0F);
            }
        }
    }
#endif
}    // namespace Board