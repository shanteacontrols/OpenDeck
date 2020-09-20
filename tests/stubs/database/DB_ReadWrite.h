#pragma once

#include <inttypes.h>
#include <string.h>
#include "dbms/src/LESSDB.h"
#include "EmuEEPROM/src/EmuEEPROM.h"

class DBstorageMock : public LESSDB::StorageAccess
{
    public:
    DBstorageMock() {}

    bool init() override
    {
#ifdef STM32_EMU_EEPROM
        emuEEPROM.init();

        for (size_t i = 0; i < (DATABASE_SIZE / 4) - 1; i++)
            eepromMemory[i] = 0xFFFF;
#endif
        return true;
    }

    uint32_t size() override;
    size_t   paramUsage(LESSDB::sectionParameterType_t type) override;
    bool     clear() override;
    bool     read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type) override;
    bool     write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type) override;

    private:
#ifdef STM32_EMU_EEPROM
    class EmuEEPROMStorageAccess : public EmuEEPROM::StorageAccess
    {
        public:
        EmuEEPROMStorageAccess() {}

        bool init() override
        {
            return true;
        }

        uint32_t startAddress(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::page1)
                return 0;
            else
                return DATABASE_SIZE;
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::page1)
                memset(pageArray, 0xFF, DATABASE_SIZE);
            else
                memset(&pageArray[DATABASE_SIZE], 0xFF, DATABASE_SIZE);

            return true;
        }

        bool write16(uint32_t address, uint16_t data) override
        {
            pageArray[address + 0] = (data >> 0) & (uint16_t)0xFF;
            pageArray[address + 1] = (data >> 8) & (uint16_t)0xFF;

            return true;
        }

        bool write32(uint32_t address, uint32_t data) override
        {
            pageArray[address + 0] = (data >> 0) & (uint32_t)0xFF;
            pageArray[address + 1] = (data >> 8) & (uint32_t)0xFF;
            pageArray[address + 2] = (data >> 16) & (uint32_t)0xFF;
            pageArray[address + 3] = (data >> 24) & (uint32_t)0xFF;

            return true;
        }

        bool read16(uint32_t address, uint16_t& data) override
        {
            data = pageArray[address + 1];
            data <<= 8;
            data |= pageArray[address + 0];

            return true;
        }

        bool read32(uint32_t address, uint32_t& data) override
        {
            data = pageArray[address + 3];
            data <<= 8;
            data |= pageArray[address + 2];
            data <<= 8;
            data |= pageArray[address + 1];
            data <<= 8;
            data |= pageArray[address + 0];

            return true;
        }

        uint32_t pageSize() override
        {
            return DATABASE_SIZE;
        }

        private:
        uint8_t pageArray[DATABASE_SIZE * 2];
    };

    EmuEEPROMStorageAccess storageMock;
    EmuEEPROM              emuEEPROM = EmuEEPROM(storageMock, false);
    //for caching - avoids constant lookups by EmuEEPROM
    //this is also used in actual firmware
    uint16_t eepromMemory[(DATABASE_SIZE / 4) - 1];
#else
    uint8_t memoryArray[DATABASE_SIZE] = {};
#endif
};