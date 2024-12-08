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

#include "application/database/database.h"
#include "application/database/layout.h"
#include "lib/emueeprom/emueeprom.h"

// Here, generated MCU header for real MCU is included, however,
// this application actually links with stub MCU. Because of this
// multiple definition errors will occur unless this header is namespaced.

namespace flashgen
{
#include <core_mcu_generated.h>
}

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iterator>
#include <string>

namespace
{
    class HwaEmuEeprom : public lib::emueeprom::Hwa
    {
        public:
        HwaEmuEeprom()
        {
            std::fill(_pageArray.at(0).begin(), _pageArray.at(0).end(), 0xFF);
            std::fill(_pageArray.at(1).begin(), _pageArray.at(1).end(), 0xFF);
        }

        bool init() override
        {
            return true;
        }

        bool erasePage(lib::emueeprom::page_t page) override
        {
            if (page == lib::emueeprom::page_t::PAGE_FACTORY)
            {
                return false;
            }

            switch (page)
            {
            case lib::emueeprom::page_t::PAGE_1:
            {
                std::fill(_pageArray.at(0).begin(), _pageArray.at(0).end(), 0xFF);
            }
            break;

            case lib::emueeprom::page_t::PAGE_2:
            {
                std::fill(_pageArray.at(1).begin(), _pageArray.at(1).end(), 0xFF);
            }
            break;

            default:
                break;
            }

            return true;
        }

        bool write32(lib::emueeprom::page_t page, uint32_t offset, uint32_t data) override
        {
            if (page == lib::emueeprom::page_t::PAGE_FACTORY)
            {
                return false;
            }

            if (offset == 0)
            {
                if (
                    (data == static_cast<uint32_t>(lib::emueeprom::pageStatus_t::RECEIVING)) ||
                    (data == static_cast<uint32_t>(lib::emueeprom::pageStatus_t::VALID)))
                {
                    _activePageWrite = page;
                }
            }

            auto& ref = page == lib::emueeprom::page_t::PAGE_1 ? _pageArray.at(0) : _pageArray.at(1);

            ref.at(offset + 0) = data >> 0 & static_cast<uint16_t>(0xFF);
            ref.at(offset + 1) = data >> 8 & static_cast<uint16_t>(0xFF);
            ref.at(offset + 2) = data >> 16 & static_cast<uint16_t>(0xFF);
            ref.at(offset + 3) = data >> 24 & static_cast<uint16_t>(0xFF);

            return true;
        }

        bool read32(lib::emueeprom::page_t page, uint32_t offset, uint32_t& data) override
        {
            // no factory page here
            if (page == lib::emueeprom::page_t::PAGE_FACTORY)
            {
                return false;
            }

            auto& ref = page == lib::emueeprom::page_t::PAGE_1 ? _pageArray.at(0) : _pageArray.at(1);

            data = ref.at(offset + 3);
            data <<= 8;
            data |= ref.at(offset + 2);
            data <<= 8;
            data |= ref.at(offset + 1);
            data <<= 8;
            data |= ref.at(offset + 0);

            return true;
        }

        void setFilename(const char* filename)
        {
            _filename = filename;
        }

        bool writeToFile()
        {
            std::fstream file;
            std::string  BIN_SUFFIX = ".bin";

            file.open(std::string(_filename + BIN_SUFFIX).c_str(), std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
            file.unsetf(std::ios::skipws);

            if (file.is_open())
            {
                size_t size = 0;
                auto&  ref  = _activePageWrite == lib::emueeprom::page_t::PAGE_1 ? _pageArray.at(0) : _pageArray.at(1);

                // get actual size of vector by finding first entry with content 0xFFFFFFFF
                for (; size < ref.size(); size += 4)
                {
                    uint32_t data = 0;

                    if (!read32(_activePageWrite, size, data))
                    {
                        while (1)
                        {
                            // should never happen
                        }
                    }

                    if (data == 0xFFFFFFFF)
                    {
                        // last entry found
                        size += 4;
                        break;
                    }
                }

                file.write(reinterpret_cast<char*>(&ref[0]), (size + 4) * sizeof(uint8_t));
                file.close();
            }
            else
            {
                return false;
            }

            // convert to hex
            std::string cmd = "srec_cat " +
                              _filename +
                              BIN_SUFFIX +
                              " -binary -offset " +
                              std::to_string(flashgen::core::mcu::flash::pageAddress(PROJECT_MCU_FLASH_PAGE_FACTORY)) +
                              " -o " +
                              _filename +
                              " -Intel";

            return system(cmd.c_str()) == 0;
        }

        private:
        std::array<std::array<uint8_t, EMU_EEPROM_PAGE_SIZE>, 2> _pageArray;
        std::string                                              _filename;
        lib::emueeprom::page_t                                   _activePageWrite = lib::emueeprom::page_t::PAGE_1;
    } hwaEmuEeprom;

    lib::emueeprom::EmuEEPROM emuEeprom(hwaEmuEeprom, false);

    class HwaDatabase : public database::Hwa
    {
        public:
        HwaDatabase() = default;

        bool init() override
        {
            return emuEeprom.init();
        }

        uint32_t size() override
        {
            return emuEeprom.maxAddress();
        }

        bool clear() override
        {
            return emuEeprom.format();
        }

        bool read(uint32_t address, uint32_t& value, lib::lessdb::sectionParameterType_t type) override
        {
            switch (type)
            {
            case lib::lessdb::sectionParameterType_t::WORD:
            case lib::lessdb::sectionParameterType_t::BYTE:
            case lib::lessdb::sectionParameterType_t::HALF_BYTE:
            case lib::lessdb::sectionParameterType_t::BIT:
            {
                uint16_t tempData   = 0;
                auto     readStatus = emuEeprom.read(address, tempData);

                if (readStatus == lib::emueeprom::readStatus_t::OK)
                {
                    value = tempData;
                }
                else if (readStatus == lib::emueeprom::readStatus_t::NO_VAR)
                {
                    // variable with this address doesn't exist yet - set value to 0
                    value = 0;
                }
                else
                {
                    return false;
                }

                return true;
            }
            break;

            default:
                return false;
            }
        }

        bool write(uint32_t address, uint32_t value, lib::lessdb::sectionParameterType_t type) override
        {
            switch (type)
            {
            case lib::lessdb::sectionParameterType_t::WORD:
            case lib::lessdb::sectionParameterType_t::BYTE:
            case lib::lessdb::sectionParameterType_t::HALF_BYTE:
            case lib::lessdb::sectionParameterType_t::BIT:
            {
                uint16_t tempData = value;

                return emuEeprom.write(address, tempData) == lib::emueeprom::writeStatus_t::OK;
            }
            break;

            default:
                return false;
            }
        }

        constexpr bool initializeDatabase() override
        {
            return true;
        }
    } _hwaDatabase;

    database::AppLayout _layout;
    database::Admin     _database(_hwaDatabase, _layout);
}    // namespace

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << argv[0] << " ERROR: Filename for generated flash not provided" << std::endl;
        return -1;
    }

    hwaEmuEeprom.setFilename(argv[1]);

    if (_database.init())
    {
        // ensure that we have clean flash binary:
        // firmware also uses custom values after defaults have been written
        emuEeprom.pageTransfer();

        return hwaEmuEeprom.writeToFile() ? 0 : 1;
    }

    return 1;
}