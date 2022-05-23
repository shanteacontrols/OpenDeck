/*

Copyright 2015-2022 Igor Petrovic

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

#include <avr/eeprom.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include "core/src/Timing.h"
#include "core/src/MCU.h"

namespace Board::bootloader
{
    /// Location at which reboot type is written in EEPROM when initiating software reset.
    constexpr uint32_t REBOOT_VALUE_EEPROM_LOCATION = EEPROM_END;

    uint8_t magicBootValue()
    {
        return eeprom_read_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION);
    }

    void setMagicBootValue(uint8_t value)
    {
        eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, static_cast<uint8_t>(value));
    }

    void runApplication()
    {
        detail::IO::indicators::ledFlashStartup();

        __asm__ __volatile__(
            // Jump to RST vector
            "clr r30\n"
            "clr r31\n"
            "ijmp\n");
    }
}    // namespace Board::bootloader