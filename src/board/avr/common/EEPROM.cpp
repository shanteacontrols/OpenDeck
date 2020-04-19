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

#include <avr/eeprom.h>
#include <avr/io.h>
#include "board/Board.h"

namespace Board
{
    namespace eeprom
    {
        uint32_t size()
        {
            //last eeprom address stores type of firmare to boot once in bootloader
            //before that, 2 bytes are used to store application CRC
            return (E2END - 2);
        }

        void init()
        {
            //nothing to do
        }

        bool read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type)
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

        void clear(uint32_t start, uint32_t end)
        {
            for (uint32_t i = start; i < end; i++)
                eeprom_update_byte(reinterpret_cast<uint8_t*>(i), 0);
        }

        size_t paramUsage(LESSDB::sectionParameterType_t type)
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::word:
                return 2;

            case LESSDB::sectionParameterType_t::dword:
                return 4;

            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::halfByte:
            case LESSDB::sectionParameterType_t::byte:
            default:
                return 1;
            }
        }
    }    // namespace eeprom
}    // namespace Board
