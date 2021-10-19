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

#include <avr/eeprom.h>
#include "board/Board.h"
#include <MCU.h>

namespace Board
{
    namespace NVM
    {
        bool init()
        {
            // nothing to do
            return true;
        }

        uint32_t size()
        {
            // last eeprom address stores type of firmware to boot once in bootloader
            return EEPROM_END;
        }

        bool read(uint32_t address, int32_t& value, parameterType_t type)
        {
            switch (type)
            {
            case parameterType_t::word:
            {
                value = eeprom_read_word(reinterpret_cast<uint16_t*>(address));
            }
            break;

            case parameterType_t::dword:
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

        bool write(uint32_t address, int32_t value, parameterType_t type)
        {
            switch (type)
            {
            case parameterType_t::word:
            {
                eeprom_update_word(reinterpret_cast<uint16_t*>(address), value);
            }
            break;

            case parameterType_t::dword:
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
                eeprom_update_byte(reinterpret_cast<uint8_t*>(i), 0);

            return true;
        }

        size_t paramUsage(parameterType_t type)
        {
            switch (type)
            {
            case parameterType_t::word:
                return 2;

            case parameterType_t::dword:
                return 4;

            default:
                return 1;
            }
        }
    }    // namespace NVM
}    // namespace Board
