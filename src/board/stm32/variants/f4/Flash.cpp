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
            bool isInRange(uint32_t address)
            {
                return IS_FLASH_ADDRESS(address);
            }

            uint32_t size()
            {
                return FLASH_BASE - FLASH_END + static_cast<uint32_t>(1);
            }

            uint32_t pageSize(size_t index)
            {
                return detail::map::flashPageDescriptor(index).size;
            }

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

            _RAM void writePage(size_t index)
            {
                //nothing to do here
            }

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

            bool read8(uint32_t address, uint8_t& data)
            {
                data = (*(volatile uint8_t*)address);
                return true;
            }

            bool read16(uint32_t address, uint16_t& data)
            {
                data = (*(volatile uint16_t*)address);
                return true;
            }

            bool read32(uint32_t address, uint32_t& data)
            {
                data = (*(volatile uint32_t*)address);
                return true;
            }
        }    // namespace flash
    }        // namespace detail
}    // namespace Board