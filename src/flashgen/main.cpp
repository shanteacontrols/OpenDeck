#include <iostream>
#include <fstream>
#include <vector>
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
                file.write(reinterpret_cast<char*>(&_flashVector[0]), _flashVector.size() * sizeof(uint8_t));
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
            _flashVector.resize(pageSize(), 0xFF);
            return true;
        }

        uint32_t startAddress(EmuEEPROM::page_t page) override
        {
            return 0;
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            return true;
        }

        bool write16(uint32_t address, uint16_t data) override
        {
            _flashVector.at(address + 0) = data >> 0 & static_cast<uint16_t>(0xFF);
            _flashVector.at(address + 1) = data >> 8 & static_cast<uint16_t>(0xFF);

            return true;
        }

        bool write32(uint32_t address, uint32_t data) override
        {
            _flashVector.at(address + 0) = data >> 0 & static_cast<uint16_t>(0xFF);
            _flashVector.at(address + 1) = data >> 8 & static_cast<uint16_t>(0xFF);
            _flashVector.at(address + 2) = data >> 16 & static_cast<uint16_t>(0xFF);
            _flashVector.at(address + 3) = data >> 24 & static_cast<uint16_t>(0xFF);

            return true;
        }

        bool read16(uint32_t address, uint16_t& data) override
        {
            if (address >= _flashVector.size())
            {
                data = 0xFFFF;
            }
            else
            {
                data = _flashVector.at(address + 1);
                data <<= 8;
                data |= _flashVector.at(address + 0);
            }

            return true;
        }

        bool read32(uint32_t address, uint32_t& data) override
        {
            if (address >= _flashVector.size())
            {
                data = 0xFFFF;
            }
            else
            {
                data = _flashVector.at(address + 3);
                data <<= 8;
                data |= _flashVector.at(address + 2);
                data <<= 8;
                data |= _flashVector.at(address + 1);
                data <<= 8;
                data |= _flashVector.at(address + 0);
            }

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
        std::vector<uint8_t> _flashVector;
        std::string          _filename;

    } emuEEPROMstorage;

    EmuEEPROM emuEEPROM(emuEEPROMstorage, false);

    class DBhandlers : public Database::Handlers
    {
        public:
        DBhandlers() = default;

        void presetChange(uint8_t preset) override
        {
        }

        void factoryResetStart() override
        {
        }

        void factoryResetDone() override
        {
        }

        void initialized() override
        {
        }
    } dbHandlers;

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
            return emuEEPROMstorage.pageSize();
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
            {
                uint16_t tempData;

                if (emuEEPROM.read(address, tempData) == EmuEEPROM::readStatus_t::ok)
                {
                    value = tempData;
                    return true;
                }

                return false;
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

    Database database(dbHandlers, storageAccess, true);
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

    return !database.init();
}