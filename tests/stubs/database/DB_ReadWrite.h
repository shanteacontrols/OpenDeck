#pragma once

#include <inttypes.h>
#include <string.h>
#include <array>
#include "dbms/src/LESSDB.h"
#include "EmuEEPROM/src/EmuEEPROM.h"
#include "board/Board.h"
#include "board/Internal.h"
#include "MCU.h"

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
        EmuEEPROMStorageAccess();

        bool     init() override;
        uint32_t startAddress(EmuEEPROM::page_t page) override;
        bool     erasePage(EmuEEPROM::page_t page) override;
        bool     write16(uint32_t address, uint16_t data) override;
        bool     write32(uint32_t address, uint32_t data) override;
        bool     read16(uint32_t address, uint16_t& data) override;
        bool     read32(uint32_t address, uint32_t& data) override;
        uint32_t pageSize() override;

        private:
        std::vector<uint8_t> pageArray;
    };

    EmuEEPROMStorageAccess storageMock;
    EmuEEPROM              emuEEPROM = EmuEEPROM(storageMock, false);
#else
    std::array<uint8_t, EEPROM_END> memoryArray;
#endif
};