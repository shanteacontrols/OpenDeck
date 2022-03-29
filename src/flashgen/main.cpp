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

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iterator>
#include <string>
#include "database/Database.h"
#include "database/Layout.h"
#include "EmuEEPROM/src/EmuEEPROM.h"

namespace
{
    class EmuEEPROMStorage : public EmuEEPROM::StorageAccess
    {
        public:
        EmuEEPROMStorage() = default;

        bool init() override
        {
            _flashVector.at(0).resize(EMU_EEPROM_PAGE_SIZE, 0xFF);
            _flashVector.at(1).resize(EMU_EEPROM_PAGE_SIZE, 0xFF);

            return true;
        }

        uint32_t startAddress(EmuEEPROM::page_t page) override
        {
            return EMU_EEPROM_PAGE_SIZE * static_cast<uint32_t>(page);
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            switch (page)
            {
            case EmuEEPROM::page_t::PAGE_2:
            {
                std::fill(_flashVector.at(1).begin(), _flashVector.at(1).end(), 0xFF);
            }
            break;

            default:
            {
                std::fill(_flashVector.at(0).begin(), _flashVector.at(0).end(), 0xFF);
            }
            break;
            }
            return true;
        }

        bool write32(uint32_t address, uint32_t data) override
        {
            size_t page = 0;

            if (address >= EMU_EEPROM_PAGE_SIZE)
            {
                address -= EMU_EEPROM_PAGE_SIZE;
                page = 1;
            }

            if (address == 0)
            {
                if (
                    (data == static_cast<uint32_t>(EmuEEPROM::pageStatus_t::RECEIVING)) ||
                    (data == static_cast<uint32_t>(EmuEEPROM::pageStatus_t::VALID)))
                {
                    _activePageWrite = page;
                }
            }

            _flashVector.at(page).at(address + 0) = data >> 0 & static_cast<uint16_t>(0xFF);
            _flashVector.at(page).at(address + 1) = data >> 8 & static_cast<uint16_t>(0xFF);
            _flashVector.at(page).at(address + 2) = data >> 16 & static_cast<uint16_t>(0xFF);
            _flashVector.at(page).at(address + 3) = data >> 24 & static_cast<uint16_t>(0xFF);

            return true;
        }

        bool read32(uint32_t address, uint32_t& data) override
        {
            size_t page = 0;

            if (address >= EMU_EEPROM_PAGE_SIZE)
            {
                address -= EMU_EEPROM_PAGE_SIZE;
                page = 1;
            }

            data = _flashVector.at(page).at(address + 3);
            data <<= 8;
            data |= _flashVector.at(page).at(address + 2);
            data <<= 8;
            data |= _flashVector.at(page).at(address + 1);
            data <<= 8;
            data |= _flashVector.at(page).at(address + 0);

            return true;
        }

        void setFilename(const char* filename)
        {
            _filename = filename;
        }

        bool writeToFile()
        {
            std::fstream file;

            file.open(_filename.c_str(), std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
            file.unsetf(std::ios::skipws);

            if (file.is_open())
            {
                size_t size = 0;

                // get actual size of vector by finding first entry with content 0xFFFFFFFF
                for (; size < _flashVector.at(_activePageWrite).size(); size += 4)
                {
                    uint32_t data;

                    if (!read32(size + (EMU_EEPROM_PAGE_SIZE * _activePageWrite), data))
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

                file.write(reinterpret_cast<char*>(&_flashVector.at(_activePageWrite)[0]), (size + 4) * sizeof(uint8_t));
                file.close();
            }
            else
            {
                return false;
            }

            _filename += "_offset";

            // also create a file containing the offset at which to write generated flash
            file.open(_filename.c_str(), std::ios::trunc | std::ios::out);

            if (file.is_open())
            {
                file << FLASH_PAGE_ADDRESS(FLASH_PAGE_FACTORY);
                file.close();

                return true;
            }

            return false;
        }

        private:
        std::array<std::vector<uint8_t>, 2> _flashVector;
        std::string                         _filename;
        size_t                              _activePageWrite = 0;

    } _emuEEPROMstorage;

    EmuEEPROM _emuEEPROM(_emuEEPROMstorage, false);

    class StorageAccess : public LESSDB::StorageAccess
    {
        public:
        StorageAccess() = default;

        bool init() override
        {
            return _emuEEPROM.init();
        }

        uint32_t size() override
        {
            return _emuEEPROM.maxAddress();
        }

        bool clear() override
        {
            return _emuEEPROM.format();
        }

        bool read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type) override
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::WORD:
            case LESSDB::sectionParameterType_t::BYTE:
            case LESSDB::sectionParameterType_t::HALF_BYTE:
            case LESSDB::sectionParameterType_t::BIT:
            {
                uint16_t tempData;

                auto readStatus = _emuEEPROM.read(address, tempData);

                if (readStatus == EmuEEPROM::readStatus_t::OK)
                {
                    value = tempData;
                }
                else if (readStatus == EmuEEPROM::readStatus_t::NO_VAR)
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

        bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type) override
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::WORD:
            case LESSDB::sectionParameterType_t::BYTE:
            case LESSDB::sectionParameterType_t::HALF_BYTE:
            case LESSDB::sectionParameterType_t::BIT:
            {
                uint16_t tempData = value;

                return _emuEEPROM.write(address, tempData) == EmuEEPROM::writeStatus_t::OK;
            }
            break;

            default:
                return false;
            }
        }
    } _storageAccess;

    Database::AppLayout _layout;
    Database::Instance  _database(_storageAccess, _layout, true);
}    // namespace

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << argv[0] << "ERROR: Filename for generated flash not provided" << std::endl;
        return -1;
    }

    _emuEEPROMstorage.setFilename(argv[1]);

    if (_database.init())
    {
        // ensure that we have clean flash binary:
        // firmware also uses custom values after defaults have been written
        _emuEEPROM.pageTransfer();

        return _emuEEPROMstorage.writeToFile() ? 0 : 1;
    }

    return 1;
}