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
#include "Internal.h"
#include "EmuEEPROM/EmuEEPROM.h"
#include "core/Timing.h"
#include "core/MCU.h"

namespace
{
    class EmuEEPROMHWA : public EmuEEPROM::StorageAccess
    {
        public:
        EmuEEPROMHWA() = default;

        bool init() override
        {
            return true;
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            switch (page)
            {
            case EmuEEPROM::page_t::PAGE_FACTORY:
                return false;

            case EmuEEPROM::page_t::PAGE_2:
            {
                for (size_t i = 0; i < PROJECT_MCU_FLASH_PAGE_EEPROM_2 - PROJECT_MCU_FLASH_PAGE_EEPROM_1; i++)
                {
                    if (!core::mcu::flash::erasePage(PROJECT_MCU_FLASH_PAGE_EEPROM_2 + i))
                    {
                        return false;
                    }
                }

                return true;
            }
            break;

            default:
            {
                for (size_t i = 0; i < PROJECT_MCU_FLASH_PAGE_EEPROM_2 - PROJECT_MCU_FLASH_PAGE_EEPROM_1; i++)
                {
                    if (!core::mcu::flash::erasePage(PROJECT_MCU_FLASH_PAGE_EEPROM_1 + i))
                    {
                        return false;
                    }
                }

                return true;
            }
            break;
            }
        }

        bool write32(EmuEEPROM::page_t page, uint32_t offset, uint32_t data) override
        {
            return core::mcu::flash::write32(START_ADDRESS(page) + offset, data);
        }

        bool read32(EmuEEPROM::page_t page, uint32_t offset, uint32_t& data) override
        {
            return core::mcu::flash::read32(START_ADDRESS(page) + offset, data);
        }

        private:
        static constexpr uint32_t START_ADDRESS(EmuEEPROM::page_t page)
        {
            switch (page)
            {
            case EmuEEPROM::page_t::PAGE_FACTORY:
                return core::mcu::flash::pageAddress(PROJECT_MCU_FLASH_PAGE_FACTORY);

            case EmuEEPROM::page_t::PAGE_2:
                return core::mcu::flash::pageAddress(PROJECT_MCU_FLASH_PAGE_EEPROM_2);

            default:
                return core::mcu::flash::pageAddress(PROJECT_MCU_FLASH_PAGE_EEPROM_1);
            }
        }
    };

    EmuEEPROMHWA _emuEEPROMHWA;
    EmuEEPROM    _emuEEPROM(_emuEEPROMHWA, true);
}    // namespace

namespace board::nvm
{
    bool init()
    {
        return _emuEEPROM.init();
    }

    uint32_t size()
    {
        return _emuEEPROM.maxAddress();
    }

    bool read(uint32_t address, uint32_t& value, parameterType_t type)
    {
        uint16_t tempData;

        switch (type)
        {
        case parameterType_t::BYTE:
        case parameterType_t::WORD:
        {
            auto readStatus = _emuEEPROM.read(address, tempData);
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

    bool write(uint32_t address, uint32_t value, parameterType_t type, bool cacheOnly)
    {
        uint16_t tempData;

        switch (type)
        {
        case parameterType_t::BYTE:
        case parameterType_t::WORD:
        {
            tempData = value;

            if (_emuEEPROM.write(address, tempData, cacheOnly) != EmuEEPROM::writeStatus_t::OK)
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
        return _emuEEPROM.format();
    }

    void writeCacheToFlash()
    {
        _emuEEPROM.writeCacheToFlash();
    }
}    // namespace board::nvm