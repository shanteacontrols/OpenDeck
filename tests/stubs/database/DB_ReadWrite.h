#pragma once

#include <inttypes.h>
#include <string.h>
#include "dbms/src/LESSDB.h"
#include "EmuEEPROM/src/EmuEEPROM.h"
#include "board/Board.h"
#include "board/Internal.h"

class DBstorageMock : public LESSDB::StorageAccess
{
    public:
    DBstorageMock() {}

    bool init() override
    {
#ifdef STM32_EMU_EEPROM
        emuEEPROM.init();
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
            pageArray.resize(pageSize() * 2, 0xFF);
            return true;
        }

        uint32_t startAddress(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::page1)
                return 0;
            else
                return pageSize();
        }

        bool erasePage(EmuEEPROM::page_t page) override
        {
            if (page == EmuEEPROM::page_t::page1)
                std::fill(pageArray.begin(), pageArray.end() - pageSize(), 0xFF);
            else
                std::fill(pageArray.begin() + pageSize(), pageArray.end(), 0xFF);

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
            return Board::detail::map::flashPageDescriptor(Board::detail::map::eepromFlashPage1()).size;
        }

        private:
        std::vector<uint8_t> pageArray;
    };

    EmuEEPROMStorageAccess storageMock;
    EmuEEPROM              emuEEPROM = EmuEEPROM(storageMock, false);
#else
    uint8_t memoryArray[DATABASE_SIZE] = {};
#endif
};