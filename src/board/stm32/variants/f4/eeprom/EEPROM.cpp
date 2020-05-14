#include "board/Board.h"
#include "board/Internal.h"
#include "EmuEEPROM/src/EmuEEPROM.h"
#include "stm32f4xx.h"
#include "Constants.h"

namespace
{
    class STM32F4EEPROM : public EmuEEPROM::StorageAccess
    {
        public:
        STM32F4EEPROM(pageDescriptor_t& page1, pageDescriptor_t& page2, size_t totalSize)
            : page1(page1)
            , page2(page2)
            , totalSize(totalSize)
        {}

        uint32_t startAddress(page_t page) override
        {
            if (page == page_t::page1)
                return page1.startAddress;
            else
                return page2.startAddress;
        }

        bool erasePage(page_t page) override
        {
            FLASH_EraseInitTypeDef pEraseInit = {};

            pEraseInit.Banks     = FLASH_BANK_1;
            pEraseInit.NbSectors = 1;

            if (page == page_t::page1)
                pEraseInit.Sector = page1.sector;
            else
                pEraseInit.Sector = page2.sector;

            pEraseInit.VoltageRange = EEPROM_VOLTAGE_RANGE;
            pEraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;

            uint32_t          eraseStatus;
            HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

            if (halStatus == HAL_OK)
            {
                __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
                halStatus = HAL_FLASHEx_Erase(&pEraseInit, &eraseStatus);
            }

            HAL_FLASH_Lock();
            return (halStatus == HAL_OK) && (eraseStatus == 0xFFFFFFFFU);
        }

        bool write16(uint32_t address, uint16_t data) override
        {
            HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

            if (halStatus == HAL_OK)
            {
                __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
                halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);
            }

            HAL_FLASH_Lock();
            return (halStatus == HAL_OK);
        }

        bool write32(uint32_t address, uint32_t data) override
        {
            HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

            if (halStatus == HAL_OK)
            {
                __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
                halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
            }

            HAL_FLASH_Lock();
            return (halStatus == HAL_OK);
        }

        bool read16(uint32_t address, uint16_t& data) override
        {
            data = (*(volatile uint16_t*)address);
            return true;
        }

        bool read32(uint32_t address, uint32_t& data) override
        {
            data = (*(volatile uint32_t*)address);
            return true;
        }

        EmuEEPROM::pageStatus_t pageStatus(page_t page) override
        {
            uint32_t status;

            if (page == page_t::page1)
                status = (*(volatile uint32_t*)page1.startAddress);
            else
                status = (*(volatile uint32_t*)page2.startAddress);

            switch (status)
            {
            case static_cast<uint32_t>(EmuEEPROM::pageStatus_t::receiving):
                return EmuEEPROM::pageStatus_t::receiving;

            case static_cast<uint32_t>(EmuEEPROM::pageStatus_t::valid):
                return EmuEEPROM::pageStatus_t::valid;

            case static_cast<uint32_t>(EmuEEPROM::pageStatus_t::erased):
            default:
                return EmuEEPROM::pageStatus_t::erased;
            }
        }

        size_t pageSize() override
        {
            return EEPROM_SIZE;
        }

        private:
        pageDescriptor_t& page1;
        pageDescriptor_t& page2;
        const size_t      totalSize;
    };

#ifdef EEPROM_RAM_CACHE
    ///
    /// \brief Memory array stored in RAM holding all the values stored in virtual EEPROM.
    /// Used to avoid constant lookups in the flash.
    ///
    uint16_t eepromMemory[(EEPROM_SIZE / 4) - 1];
#endif

    STM32F4EEPROM stm32EEPROM(Board::detail::map::eepromFlashPage1(),
                              Board::detail::map::eepromFlashPage2(),
                              EEPROM_SIZE);

    EmuEEPROM emuEEPROM(stm32EEPROM);
}    // namespace

namespace Board
{
    namespace eeprom
    {
        uint32_t size()
        {
            return EEPROM_SIZE;
        }

        void init()
        {
            emuEEPROM.init();

            for (size_t i = 0; i < (EEPROM_SIZE / 4) - 1; i++)
                eepromMemory[i] = 0xFFFF;
        }

        bool read(uint32_t address, int32_t& value, parameterType_t type)
        {
            uint16_t tempData;

            switch (type)
            {
            case parameterType_t::byte:
            case parameterType_t::word:
                if (eepromMemory[address] != 0xFFFF)
                {
                    value = eepromMemory[address];
                }
                else
                {
                    if (emuEEPROM.read(address, tempData) != EmuEEPROM::readStatus_t::ok)
                    {
                        return false;
                    }
                    else
                    {
                        value                 = tempData;
                        eepromMemory[address] = tempData;
                    }
                }
                break;

            default:
                return false;
                break;
            }

            return true;
        }

        bool write(uint32_t address, int32_t value, parameterType_t type)
        {
            uint16_t tempData;

            switch (type)
            {
            case parameterType_t::byte:
            case parameterType_t::word:
                tempData              = value;
                eepromMemory[address] = value;
                if (emuEEPROM.write(address, tempData) != EmuEEPROM::writeStatus_t::ok)
                    return false;
                break;

            default:
                return false;
                break;
            }

            return true;
        }

        void clear(uint32_t start, uint32_t end)
        {
            //ignore start/end markers on stm32 for now
            emuEEPROM.format();
        }

        size_t paramUsage(parameterType_t type)
        {
            switch (type)
            {
            case parameterType_t::dword:
                return 8;

            default:
                return 4;    //2 bytes for address, 2 bytes for data
            }
        }
    }    // namespace eeprom
}    // namespace Board