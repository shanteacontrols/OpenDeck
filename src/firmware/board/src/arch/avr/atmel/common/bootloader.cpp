/*

Copyright Igor Petrovic

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

#include "board/board.h"
#include "internal.h"

#include "core/mcu.h"
#include "core/util/util.h"

#include <avr/eeprom.h>

namespace board::bootloader
{
    /// Location at which reboot type is written in EEPROM when initiating software reset.
    constexpr uint32_t REBOOT_VALUE_EEPROM_LOCATION = CORE_MCU_EEPROM_SIZE - 4;

    uint32_t magicBootValue()
    {
        return eeprom_read_dword((uint32_t*)REBOOT_VALUE_EEPROM_LOCATION);
    }

    void setMagicBootValue(uint32_t value)
    {
        eeprom_update_dword((uint32_t*)REBOOT_VALUE_EEPROM_LOCATION, value);
    }

    void runApplication()
    {
        __asm__ __volatile__(
            // Jump to RST vector
            "clr r30\n"
            "clr r31\n"
            "ijmp\n");
    }
}    // namespace board::bootloader