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
        emuEEPROM.init();
#endif
        return true;
    }

    uint32_t size() override
    {
#ifdef EMUEEPROM_INCLUDE_CONFIG
        return emuEEPROM.maxAddress();
#else
        return memoryArray.size();
#endif
    }

    bool clear() override
    {
#ifndef EMUEEPROM_INCLUDE_CONFIG
        std::fill(memoryArray.begin(), memoryArray.end(), 0x00);
        return true;
#else
        return emuEEPROM.format();
#endif
    }

    bool read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type) override
    {
#ifndef EMUEEPROM_INCLUDE_CONFIG
        switch (type)
        {
        case LESSDB::sectionParameterType_t::bit:
        case LESSDB::sectionParameterType_t::byte:
        case LESSDB::sectionParameterType_t::halfByte:
        {
            value = memoryArray.at(address);
        }
        break;

        case LESSDB::sectionParameterType_t::word:
        {
            value = memoryArray.at(address + 1);
            value <<= 8;
            value |= memoryArray.at(address + 0);
        }
        break;

        default:
        {
            // case LESSDB::sectionParameterType_t::dword:
            value = memoryArray.at(address + 3);
            value <<= 8;
            value |= memoryArray.at(address + 2);
            value <<= 8;
            value |= memoryArray.at(address + 1);
            value <<= 8;
            value |= memoryArray.at(address + 0);
        }
        break;
        }

        return true;
#else
        uint16_t tempData;

        switch (type)
        {
        case LESSDB::sectionParameterType_t::bit:
        case LESSDB::sectionParameterType_t::byte:
        case LESSDB::sectionParameterType_t::halfByte:
        case LESSDB::sectionParameterType_t::word:
        {
            auto readStatus = emuEEPROM.read(address, tempData);

            if (readStatus == EmuEEPROM::readStatus_t::ok)
            {
                value = tempData;
            }
            else if (readStatus == EmuEEPROM::readStatus_t::noVar)
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
        case LESSDB::sectionParameterType_t::bit:
        case LESSDB::sectionParameterType_t::byte:
        case LESSDB::sectionParameterType_t::halfByte:
        {
            memoryArray.at(address) = value;
        }
        break;

        case LESSDB::sectionParameterType_t::word:
        {
            memoryArray.at(address + 0) = (value >> 0) & (uint16_t)0xFF;
            memoryArray.at(address + 1) = (value >> 8) & (uint16_t)0xFF;
        }
        break;

        default:
        {
            // case LESSDB::sectionParameterType_t::dword:
            memoryArray.at(address + 0) = (value >> 0) & (uint32_t)0xFF;
            memoryArray.at(address + 1) = (value >> 8) & (uint32_t)0xFF;
            memoryArray.at(address + 2) = (value >> 16) & (uint32_t)0xFF;
            memoryArray.at(address + 3) = (value >> 24) & (uint32_t)0xFF;
        }
        break;
        }

        return true;
#else
        uint16_t tempData;

        switch (type)
        {
        case LESSDB::sectionParameterType_t::bit:
        case LESSDB::sectionParameterType_t::byte:
        case LESSDB::sectionParameterType_t::halfByte:
        case LESSDB::sectionParameterType_t::word:
        {
            tempData = value;

            if (emuEEPROM.write(address, tempData) != EmuEEPROM::writeStatus_t::ok)
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
            pageArray.resize(EMU_EEPROM_PAGE_SIZE * 2, 0xFF);
        }

        bool init() override
        {
            return true;
        }

        uint32_t startAddress(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::page1)
                return 0;
            else
                return EMU_EEPROM_PAGE_SIZE;
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::page1)
                std::fill(pageArray.begin(), pageArray.end() - EMU_EEPROM_PAGE_SIZE, 0xFF);
            else
                std::fill(pageArray.begin() + EMU_EEPROM_PAGE_SIZE, pageArray.end(), 0xFF);

            return true;
        }

        bool write16(uint32_t address, uint16_t data) override
        {
            // 0->1 transition is not allowed
            uint16_t currentData = 0;
            read16(address, currentData);

            if (data > currentData)
                return false;

            pageArray.at(address + 0) = (data >> 0) & (uint16_t)0xFF;
            pageArray.at(address + 1) = (data >> 8) & (uint16_t)0xFF;

            return true;
        }

        bool write32(uint32_t address, uint32_t data) override
        {
            // 0->1 transition is not allowed
            uint32_t currentData = 0;
            read32(address, currentData);

            if (data > currentData)
                return false;

            pageArray.at(address + 0) = (data >> 0) & (uint32_t)0xFF;
            pageArray.at(address + 1) = (data >> 8) & (uint32_t)0xFF;
            pageArray.at(address + 2) = (data >> 16) & (uint32_t)0xFF;
            pageArray.at(address + 3) = (data >> 24) & (uint32_t)0xFF;

            return true;
        }

        bool read16(uint32_t address, uint16_t& data) override
        {
            data = pageArray.at(address + 1);
            data <<= 8;
            data |= pageArray.at(address + 0);

            return true;
        }

        bool read32(uint32_t address, uint32_t& data) override
        {
            data = pageArray.at(address + 3);
            data <<= 8;
            data |= pageArray.at(address + 2);
            data <<= 8;
            data |= pageArray.at(address + 1);
            data <<= 8;
            data |= pageArray.at(address + 0);

            return true;
        }

        private:
        std::vector<uint8_t> pageArray;
    };

    EmuEEPROMStorageAccess storageMock;
    EmuEEPROM              emuEEPROM = EmuEEPROM(storageMock, false);
#else
    std::array<uint8_t, EEPROM_END> memoryArray = {};
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