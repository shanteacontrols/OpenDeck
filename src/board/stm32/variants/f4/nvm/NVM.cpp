#include "board/Board.h"
#include "board/Internal.h"
#include "stm32f4xx.h"

#define _RAM __attribute__((section(".data#"), noinline))

namespace Board
{
    namespace detail
    {
        namespace flash
        {
            ///
            /// \brief Erases specified flash page.
            ///
            _RAM bool erasePage(size_t index)
            {
                FLASH_EraseInitTypeDef pEraseInit = {};

                pEraseInit.Banks        = FLASH_BANK_1;
                pEraseInit.NbSectors    = 1;
                pEraseInit.Sector       = index;
                pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
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

            ///
            /// \brief Write 16-bit data to specified address in flash memory.
            ///
            _RAM bool write16(uint32_t address, uint16_t data)
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

            _RAM bool write16(uint32_t address, uint16_t* data, uint32_t count)
            {
                HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

                if (halStatus == HAL_OK)
                {
                    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

                    for (uint32_t i = 0; i < count; i++)
                    {
                        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + (i * 2), data[i]) != HAL_OK)
                            return false;
                    }
                }

                HAL_FLASH_Lock();
                return (halStatus == HAL_OK);
            }

            ///
            /// \brief Write 32-bit data to specified address in flash memory.
            ///
            _RAM bool write32(uint32_t address, uint32_t data)
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

            _RAM bool write32(uint32_t address, uint32_t* data, uint32_t count)
            {
                HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

                if (halStatus == HAL_OK)
                {
                    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

                    for (uint32_t i = 0; i < count; i++)
                    {
                        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + (i * 4), data[i]) != HAL_OK)
                            return false;
                    }
                }

                HAL_FLASH_Lock();
                return (halStatus == HAL_OK);
            }

            ///
            /// \brief Read 16-bit data from specified address in flash memory.
            ///
            bool read16(uint32_t address, uint16_t& data)
            {
                data = (*(volatile uint16_t*)address);
                return true;
            }

            ///
            /// \brief Read 32-bit data from specified address in flash memory.
            ///
            bool read32(uint32_t address, uint32_t& data)
            {
                data = (*(volatile uint32_t*)address);
                return true;
            }
        }    // namespace flash
    }        // namespace detail
}    // namespace Board