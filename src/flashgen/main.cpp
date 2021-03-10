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

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iterator>
#include <string>
#include "database/Database.h"
#include "EmuEEPROM/src/EmuEEPROM.h"
#include "board/Internal.h"

namespace
{
    class EmuEEPROMStorage : public EmuEEPROM::StorageAccess
    {
        public:
        EmuEEPROMStorage() = default;

        ~EmuEEPROMStorage()
        {
            std::fstream file;

            file.open(_filename.c_str(), std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
            file.unsetf(std::ios::skipws);

            if (file.is_open())
            {
                size_t size = 0;

                //get actual size of vector by finding first entry with content 0xFFFFFFFF
                for (; size < _flashVector.at(_activePageWrite).size(); size += 4)
                {
                    uint32_t data;

                    if (!read32(size + (pageSize() * _activePageWrite), data))
                    {
                        while (1)
                        {
                            //should never happen
                        }
                    }

                    if (data == 0xFFFFFFFF)
                    {
                        //last entry found
                        size += 4;
                        break;
                    }
                }

                file.write(reinterpret_cast<char*>(&_flashVector.at(_activePageWrite)[0]), (size + 4) * sizeof(uint8_t));
                file.close();
            }

            _filename += "_offset";

            //also create a file containing the offset at which to write generated flash
            file.open(_filename.c_str(), std::ios::trunc | std::ios::out);

            if (file.is_open())
            {
                file << Board::detail::map::flashPageDescriptor(Board::detail::map::eepromFlashPageFactory()).address;
                file.close();
            }
        }

        bool init() override
        {
            _flashVector.at(0).resize(pageSize(), 0xFF);
            _flashVector.at(1).resize(pageSize(), 0xFF);
            return true;
        }

        uint32_t startAddress(EmuEEPROM::page_t page) override
        {
            return pageSize() * static_cast<uint32_t>(page);
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            switch (page)
            {
            case EmuEEPROM::page_t::page2:
                std::fill(_flashVector.at(1).begin(), _flashVector.at(1).end(), 0xFF);
                break;

            default:
                std::fill(_flashVector.at(0).begin(), _flashVector.at(0).end(), 0xFF);
                break;
            }
            return true;
        }

        bool write16(uint32_t address, uint16_t data) override
        {
            size_t page = 0;

            if (address >= pageSize())
            {
                address -= pageSize();
                page = 1;
            }

            _flashVector.at(page).at(address + 0) = data >> 0 & static_cast<uint16_t>(0xFF);
            _flashVector.at(page).at(address + 1) = data >> 8 & static_cast<uint16_t>(0xFF);

            return true;
        }

        bool write32(uint32_t address, uint32_t data) override
        {
            size_t page = 0;

            if (address >= pageSize())
            {
                address -= pageSize();
                page = 1;
            }

            if (address == 0)
            {
                if (
                    (data == static_cast<uint32_t>(EmuEEPROM::pageStatus_t::receiving)) ||
                    (data == static_cast<uint32_t>(EmuEEPROM::pageStatus_t::valid)))
                    _activePageWrite = page;
            }

            _flashVector.at(page).at(address + 0) = data >> 0 & static_cast<uint16_t>(0xFF);
            _flashVector.at(page).at(address + 1) = data >> 8 & static_cast<uint16_t>(0xFF);
            _flashVector.at(page).at(address + 2) = data >> 16 & static_cast<uint16_t>(0xFF);
            _flashVector.at(page).at(address + 3) = data >> 24 & static_cast<uint16_t>(0xFF);

            return true;
        }

        bool read16(uint32_t address, uint16_t& data) override
        {
            size_t page = 0;

            if (address >= pageSize())
            {
                address -= pageSize();
                page = 1;
            }

            data = _flashVector.at(page).at(address + 1);
            data <<= 8;
            data |= _flashVector.at(page).at(address + 0);

            return true;
        }

        bool read32(uint32_t address, uint32_t& data) override
        {
            size_t page = 0;

            if (address >= pageSize())
            {
                address -= pageSize();
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

        uint32_t pageSize() override
        {
            return Board::detail::map::flashPageDescriptor(Board::detail::map::eepromFlashPage1()).size;
        }

        void setFilename(const char* filename)
        {
            _filename = filename;
        }

        private:
        std::array<std::vector<uint8_t>, 2> _flashVector;
        std::string                         _filename;
        size_t                              _activePageWrite = 0;

    } emuEEPROMstorage;

    EmuEEPROM emuEEPROM(emuEEPROMstorage, false);

    class StorageAccess : public LESSDB::StorageAccess
    {
        public:
        StorageAccess() = default;

        bool init() override
        {
            return emuEEPROM.init();
        }

        uint32_t size() override
        {
            //first 4 bytes are reserved for page status
            return emuEEPROMstorage.pageSize() - 4;
        }

        bool clear() override
        {
            return emuEEPROM.format();
        }

        bool read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type) override
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::word:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
            case LESSDB::sectionParameterType_t::bit:
            {
                uint16_t tempData;

                auto readStatus = emuEEPROM.read(address, tempData);

                if (readStatus == EmuEEPROM::readStatus_t::ok)
                {
                    value = tempData;
                }
                else if (readStatus == EmuEEPROM::readStatus_t::noVar)
                {
                    //variable with this address doesn't exist yet - set value to 0
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
            case LESSDB::sectionParameterType_t::word:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
            case LESSDB::sectionParameterType_t::bit:
            {
                uint16_t tempData = value;

                return emuEEPROM.write(address, tempData) == EmuEEPROM::writeStatus_t::ok;
            }
            break;

            default:
                return false;
            }
        }

        size_t paramUsage(LESSDB::sectionParameterType_t type) override
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::dword:
                return 8;

            default:
                return 4;    //2 bytes for address, 2 bytes for data
            }
        }
    } storageAccess;

    Database database(storageAccess, true);
}    // namespace

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << argv[0] << "ERROR: Filename for generated flash not provided" << std::endl;
        return -1;
    }
    else
    {
        emuEEPROMstorage.setFilename(argv[1]);
    }

    if (database.init())
    {
        //ensure that we have clean flash binary:
        //firmware also uses custom values after defaults have been written
        emuEEPROM.pageTransfer();
        return 0;
    }

    return 1;
}