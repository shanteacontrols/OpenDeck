#pragma once

#include <MCU.h>
#ifdef EMUEEPROM_INCLUDE_CONFIG
#include "EmuEEPROM/src/EmuEEPROM.h"
#endif
#include "database/Database.h"
#include "database/Layout.h"
#include "framework/Framework.h"

class DBstorageMock : public LESSDB::StorageAccess
{
    public:
    DBstorageMock() = default;

    bool init() override
    {
#ifdef EMUEEPROM_INCLUDE_CONFIG
        _emuEEPROM.init();
#endif
        return true;
    }

    uint32_t size() override
    {
#ifdef EMUEEPROM_INCLUDE_CONFIG
        return _emuEEPROM.maxAddress();
#else
        return _memoryArray.size();
#endif
    }

    bool clear() override
    {
#ifndef EMUEEPROM_INCLUDE_CONFIG
        std::fill(_memoryArray.begin(), _memoryArray.end(), 0x00);
        return true;
#else
        return _emuEEPROM.format();
#endif
    }

    bool read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type) override
    {
#ifndef EMUEEPROM_INCLUDE_CONFIG
        switch (type)
        {
        case LESSDB::sectionParameterType_t::BIT:
        case LESSDB::sectionParameterType_t::BYTE:
        case LESSDB::sectionParameterType_t::HALF_BYTE:
        {
            value = _memoryArray.at(address);
        }
        break;

        case LESSDB::sectionParameterType_t::WORD:
        {
            value = _memoryArray.at(address + 1);
            value <<= 8;
            value |= _memoryArray.at(address + 0);
        }
        break;

        default:
        {
            // case LESSDB::sectionParameterType_t::DWORD:
            value = _memoryArray.at(address + 3);
            value <<= 8;
            value |= _memoryArray.at(address + 2);
            value <<= 8;
            value |= _memoryArray.at(address + 1);
            value <<= 8;
            value |= _memoryArray.at(address + 0);
        }
        break;
        }

        return true;
#else
        uint16_t tempData;

        switch (type)
        {
        case LESSDB::sectionParameterType_t::BIT:
        case LESSDB::sectionParameterType_t::BYTE:
        case LESSDB::sectionParameterType_t::HALF_BYTE:
        case LESSDB::sectionParameterType_t::WORD:
        {
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
        }
        break;

        default:
            return false;
        }

        return true;
#endif
    }

    bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type) override
    {
#ifndef EMUEEPROM_INCLUDE_CONFIG
        switch (type)
        {
        case LESSDB::sectionParameterType_t::BIT:
        case LESSDB::sectionParameterType_t::BYTE:
        case LESSDB::sectionParameterType_t::HALF_BYTE:
        {
            _memoryArray.at(address) = value;
        }
        break;

        case LESSDB::sectionParameterType_t::WORD:
        {
            _memoryArray.at(address + 0) = (value >> 0) & (uint16_t)0xFF;
            _memoryArray.at(address + 1) = (value >> 8) & (uint16_t)0xFF;
        }
        break;

        default:
        {
            // case LESSDB::sectionParameterType_t::DWORD:
            _memoryArray.at(address + 0) = (value >> 0) & (uint32_t)0xFF;
            _memoryArray.at(address + 1) = (value >> 8) & (uint32_t)0xFF;
            _memoryArray.at(address + 2) = (value >> 16) & (uint32_t)0xFF;
            _memoryArray.at(address + 3) = (value >> 24) & (uint32_t)0xFF;
        }
        break;
        }

        return true;
#else
        uint16_t tempData;

        switch (type)
        {
        case LESSDB::sectionParameterType_t::BIT:
        case LESSDB::sectionParameterType_t::BYTE:
        case LESSDB::sectionParameterType_t::HALF_BYTE:
        case LESSDB::sectionParameterType_t::WORD:
        {
            tempData = value;

            if (_emuEEPROM.write(address, tempData) != EmuEEPROM::writeStatus_t::OK)
                return false;
        }
        break;

        default:
            return false;
        }

        return true;
#endif
    }

    private:
#ifdef EMUEEPROM_INCLUDE_CONFIG
    class EmuEEPROMStorageAccess : public EmuEEPROM::StorageAccess
    {
        public:
        EmuEEPROMStorageAccess()
        {
            _pageArray.resize(EMU_EEPROM_PAGE_SIZE * 2, 0xFF);
        }

        bool init() override
        {
            return true;
        }

        uint32_t startAddress(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::PAGE_1)
            {
                return 0;
            }

            return EMU_EEPROM_PAGE_SIZE;
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::PAGE_1)
            {
                std::fill(_pageArray.begin(), _pageArray.end() - EMU_EEPROM_PAGE_SIZE, 0xFF);
            }
            else
            {
                std::fill(_pageArray.begin() + EMU_EEPROM_PAGE_SIZE, _pageArray.end(), 0xFF);
            }

            return true;
        }

        bool write32(uint32_t address, uint32_t data) override
        {
            // 0->1 transition is not allowed
            uint32_t currentData = 0;
            read32(address, currentData);

            if (data > currentData)
            {
                return false;
            }

            _pageArray.at(address + 0) = (data >> 0) & (uint32_t)0xFF;
            _pageArray.at(address + 1) = (data >> 8) & (uint32_t)0xFF;
            _pageArray.at(address + 2) = (data >> 16) & (uint32_t)0xFF;
            _pageArray.at(address + 3) = (data >> 24) & (uint32_t)0xFF;

            return true;
        }

        bool read32(uint32_t address, uint32_t& data) override
        {
            data = _pageArray.at(address + 3);
            data <<= 8;
            data |= _pageArray.at(address + 2);
            data <<= 8;
            data |= _pageArray.at(address + 1);
            data <<= 8;
            data |= _pageArray.at(address + 0);

            return true;
        }

        private:
        std::vector<uint8_t> _pageArray;
    };

    EmuEEPROMStorageAccess _storageMock;
    EmuEEPROM              _emuEEPROM = EmuEEPROM(_storageMock, false);
#else
    std::array<uint8_t, EEPROM_END> _memoryArray = {};
#endif
};

class TestDatabase
{
    public:
    TestDatabase() = default;

    private:
    DBstorageMock       _dbStorageMock;
    Database::AppLayout _layout;

    public:
    Database::Instance _instance = Database::Instance(_dbStorageMock, _layout, true);
};