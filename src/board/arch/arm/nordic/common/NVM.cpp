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

#include "board/Board.h"
#include "board/Internal.h"
#include "EmuEEPROM/src/EmuEEPROM.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/Timing.h"
#include "board/common/constants/IO.h"
#include <MCU.h>

namespace
{
    class NRF52EEPROM : public EmuEEPROM::StorageAccess
    {
        public:
        NRF52EEPROM() = default;

        bool init() override
        {
            return true;
        }

        uint32_t startAddress(EmuEEPROM::page_t page) override
        {
            switch (page)
            {
            case EmuEEPROM::page_t::PAGE_FACTORY:
                return FLASH_PAGE_ADDRESS(FLASH_PAGE_FACTORY);

            case EmuEEPROM::page_t::PAGE_2:
                return FLASH_PAGE_ADDRESS(FLASH_PAGE_EEPROM_2);

            default:
                return FLASH_PAGE_ADDRESS(FLASH_PAGE_EEPROM_1);
            }
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            switch (page)
            {
            case EmuEEPROM::page_t::PAGE_FACTORY:
                return false;

            case EmuEEPROM::page_t::PAGE_2:
            {
                for (size_t i = 0; i < FLASH_PAGE_EEPROM_2 - FLASH_PAGE_EEPROM_1; i++)
                {
                    if (!Board::detail::flash::erasePage(FLASH_PAGE_EEPROM_2 + i))
                    {
                        return false;
                    }
                }

                return true;
            }
            break;

            default:
            {
                for (size_t i = 0; i < FLASH_PAGE_EEPROM_2 - FLASH_PAGE_EEPROM_1; i++)
                {
                    if (!Board::detail::flash::erasePage(FLASH_PAGE_EEPROM_1 + i))
                    {
                        return false;
                    }
                }

                return true;
            }
            break;
            }
        }

        bool write32(uint32_t address, uint32_t data) override
        {
            return Board::detail::flash::write32(address, data);
        }

        bool read32(uint32_t address, uint32_t& data) override
        {
            return Board::detail::flash::read32(address, data);
        }
    };

    NRF52EEPROM _stm32EEPROM;
    EmuEEPROM   _emuEEPROM(_stm32EEPROM, true);
}    // namespace

namespace Board::NVM
{
    bool init()
    {
        return _emuEEPROM.init();
    }

    uint32_t size()
    {
        return _emuEEPROM.maxAddress();
    }

    bool read(uint32_t address, int32_t& value, parameterType_t type)
    {
        uint16_t tempData;

        switch (type)
        {
        case parameterType_t::BYTE:
        case parameterType_t::WORD:
        {
            auto readStatus = _emuEEPROM.readCached(address, tempData);
            value           = tempData;

            if (readStatus == EmuEEPROM::readStatus_t::OK)
            {
                value = tempData;
            }
            else if (readStatus == EmuEEPROM::readStatus_t::NO_VAR)
            {
                // variable with this address doesn't exist yet - set value to 0
                value = 0;
            }
        }
        break;

        default:
            return false;
        }

        return true;
    }

    bool write(uint32_t address, int32_t value, parameterType_t type)
    {
        uint16_t tempData;

        switch (type)
        {
        case parameterType_t::BYTE:
        case parameterType_t::WORD:
        {
            tempData = value;

            if (_emuEEPROM.write(address, tempData) != EmuEEPROM::writeStatus_t::OK)
            {
                return false;
            }
        }

        break;

        default:
            return false;
        }

        return true;
    }

    bool clear(uint32_t start, uint32_t end)
    {
        bool result;

#ifdef LED_INDICATORS
        // Clearing is usually called in runtime so it's possible that LED
        // indicators are still on since the command is most likely given via USB.
        // Wait until all indicators are turned off.
        core::timing::waitMs(MIDI_INDICATOR_TIMEOUT);
#endif

        result = _emuEEPROM.format();

        return result;
    }
}    // namespace Board::NVM