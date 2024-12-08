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

#include "core/mcu.h"
#include <avr/eeprom.h>

namespace board::nvm
{
    bool init()
    {
        // nothing to do
        return true;
    }

    uint32_t size()
    {
        // last 4 bytes in eeprom are reserved for the type of firmware to boot once in bootloader
        return CORE_MCU_EEPROM_SIZE - 4;
    }

    bool read(uint32_t address, uint32_t& value, parameterType_t type)
    {
        switch (type)
        {
        case parameterType_t::WORD:
        {
            value = eeprom_read_word(reinterpret_cast<uint16_t*>(address));
        }
        break;

        case parameterType_t::DWORD:
        {
            value = eeprom_read_dword(reinterpret_cast<uint32_t*>(address));
        }
        break;

        default:
        {
            value = eeprom_read_byte(reinterpret_cast<uint8_t*>(address));
        }
        break;
        }

        return true;
    }

    bool write(uint32_t address, uint32_t value, parameterType_t type, bool cacheOnly)
    {
        switch (type)
        {
        case parameterType_t::WORD:
        {
            eeprom_update_word(reinterpret_cast<uint16_t*>(address), value);
        }
        break;

        case parameterType_t::DWORD:
        {
            eeprom_update_dword(reinterpret_cast<uint32_t*>(address), value);
        }
        break;

        default:
        {
            eeprom_update_byte(reinterpret_cast<uint8_t*>(address), value);
        }
        break;
        }

        return true;
    }

    bool clear(uint32_t start, uint32_t end)
    {
        for (uint32_t i = start; i < end; i++)
        {
            eeprom_update_byte(reinterpret_cast<uint8_t*>(i), 0);
        }

        return true;
    }

    void writeCacheToFlash()
    {
    }
}    // namespace board::nvm
