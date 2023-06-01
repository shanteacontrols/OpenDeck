#pragma once

#ifdef PROJECT_MCU_USE_EMU_EEPROM
#include "EmuEEPROM/EmuEEPROM.h"
#endif
#include "application/database/Layout.h"
#include "tests/Common.h"
#include "application/system/Builder.h"

class DBstorageMock : public ::sys::Builder::HWA::Database
{
    public:
    DBstorageMock() = default;

    bool init() override
    {
#ifdef PROJECT_MCU_USE_EMU_EEPROM
        _emuEEPROM.init();
#endif
        return true;
    }

    uint32_t size() override
    {
#ifdef PROJECT_MCU_USE_EMU_EEPROM
        return _emuEEPROM.maxAddress();
#else
        return _memoryArray.size();
#endif
    }

    bool clear() override
    {
#ifdef PROJECT_MCU_USE_EMU_EEPROM
        return _emuEEPROM.format();
#else
        std::fill(_memoryArray.begin(), _memoryArray.end(), 0x00);
        return true;
#endif
    }

    bool read(uint32_t address, uint32_t& value, LESSDB::sectionParameterType_t type) override
    {
#ifdef PROJECT_MCU_USE_EMU_EEPROM
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
#else
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
#endif
    }

    bool write(uint32_t address, uint32_t value, LESSDB::sectionParameterType_t type) override
    {
#ifdef PROJECT_MCU_USE_EMU_EEPROM
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
#else
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
#endif
    }

    bool initializeDatabase() override
    {
        return true;
    }

    private:
#ifdef PROJECT_MCU_USE_EMU_EEPROM
    class EmuEEPROMStorageAccess : public EmuEEPROM::StorageAccess
    {
        public:
        EmuEEPROMStorageAccess()
        {
            std::fill(_pageArray.at(0).begin(), _pageArray.at(0).end(), 0xFF);
            std::fill(_pageArray.at(1).begin(), _pageArray.at(1).end(), 0xFF);
        }

        bool init() override
        {
            return true;
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::PAGE_FACTORY)
            {
                return false;
            }

            auto& ref = page == EmuEEPROM::page_t::PAGE_1 ? _pageArray.at(0) : _pageArray.at(1);

            std::fill(ref.begin(), ref.end(), 0xFF);
            return true;
        }

        bool write32(EmuEEPROM::page_t page, uint32_t offset, uint32_t data) override
        {
            if (page == EmuEEPROM::page_t::PAGE_FACTORY)
            {
                return false;
            }

            // 0->1 transition is not allowed
            uint32_t currentData = 0;
            read32(page, offset, currentData);

            if (data > currentData)
            {
                return false;
            }

            auto& ref = page == EmuEEPROM::page_t::PAGE_1 ? _pageArray.at(0) : _pageArray.at(1);

            ref.at(offset + 0) = data >> 0 & static_cast<uint16_t>(0xFF);
            ref.at(offset + 1) = data >> 8 & static_cast<uint16_t>(0xFF);
            ref.at(offset + 2) = data >> 16 & static_cast<uint16_t>(0xFF);
            ref.at(offset + 3) = data >> 24 & static_cast<uint16_t>(0xFF);

            return true;
        }

        bool read32(EmuEEPROM::page_t page, uint32_t offset, uint32_t& data) override
        {
            // no factory page here
            if (page == EmuEEPROM::page_t::PAGE_FACTORY)
            {
                return false;
            }

            auto& ref = page == EmuEEPROM::page_t::PAGE_1 ? _pageArray.at(0) : _pageArray.at(1);

            data = ref.at(offset + 3);
            data <<= 8;
            data |= ref.at(offset + 2);
            data <<= 8;
            data |= ref.at(offset + 1);
            data <<= 8;
            data |= ref.at(offset + 0);

            return true;
        }

        private:
        std::array<std::array<uint8_t, EMU_EEPROM_PAGE_SIZE>, 2> _pageArray;
    };

    EmuEEPROMStorageAccess _storageMock;
    EmuEEPROM              _emuEEPROM = EmuEEPROM(_storageMock, false);
#else
    std::array<uint8_t, PROJECT_MCU_EEPROM_SIZE - 1> _memoryArray = {};
#endif
};

class TestDatabase
{
    public:
    TestDatabase() = default;

    private:
    DBstorageMock       _dbStorageMock;
    database::AppLayout _layout;

    public:
    database::Admin _instance = database::Admin(_dbStorageMock, _layout, true);
};