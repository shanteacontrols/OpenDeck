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

#include "board/Board.h"
#include "board/Internal.h"
#include "board/stm32/eeprom/EEPROM.h"

namespace
{
    EmuEEPROM emuEEPROM(Board::detail::map::eepromFlashPage1(), Board::detail::map::eepromFlashPage2());
}    // namespace

namespace Board
{
    namespace eeprom
    {
        void init()
        {
            emuEEPROM.init();
        }

        bool read(uint32_t address, LESSDB::sectionParameterType_t type, int32_t& value)
        {
            uint16_t tempData;

            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
            case LESSDB::sectionParameterType_t::word:
                if (emuEEPROM.read(address, tempData) != EmuEEPROM::readStatus_t::ok)
                    return false;
                else
                    value = tempData;
                break;

            default:
                return false;
                break;
            }

            return true;
        }

        bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type)
        {
            uint16_t tempData;

            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
            case LESSDB::sectionParameterType_t::word:
                tempData = value;
                if (emuEEPROM.write(address, tempData) != EmuEEPROM::writeStatus_t::ok)
                    return false;
                break;

            default:
                return false;
                break;
            }

            return true;
        }
    }    // namespace eeprom
}    // namespace Board