/*

Copyright 2015-2022 Igor Petrovic

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
#include "nrfx_nvmc.h"
#include <Target.h>

#define _RAM __attribute__((section(".data#"), noinline))

namespace Board::detail::flash
{
    bool isInRange(uint32_t address)
    {
        return address <= FLASH_END;
    }

    uint32_t size()
    {
        return FLASH_END + static_cast<uint32_t>(1);
    }

    uint32_t pageSize(size_t index)
    {
        return detail::map::flashPageDescriptor(index).size;
    }

    _RAM bool erasePage(size_t index)
    {
        return nrfx_nvmc_page_erase(FLASH_PAGE_ADDRESS(index)) == NRFX_SUCCESS;
    }

    _RAM void writePage(size_t index)
    {
        // nothing to do here
    }

    _RAM bool write16(uint32_t address, uint16_t data)
    {
        nrf_nvmc_mode_set(NRF_NVMC, NRF_NVMC_MODE_WRITE);
        nrfx_nvmc_byte_write(address + 0, data >> 0 & static_cast<uint8_t>(0xFF));
        nrfx_nvmc_byte_write(address + 1, data >> 8 & static_cast<uint8_t>(0xFF));
        nrf_nvmc_mode_set(NRF_NVMC, NRF_NVMC_MODE_READONLY);

        return (*(volatile uint16_t*)address) == data;
    }

    _RAM bool write32(uint32_t address, uint32_t data)
    {
        nrf_nvmc_mode_set(NRF_NVMC, NRF_NVMC_MODE_WRITE);
        nrfx_nvmc_word_write(address, data);
        nrf_nvmc_mode_set(NRF_NVMC, NRF_NVMC_MODE_READONLY);

        return (*(volatile uint32_t*)address) == data;
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
}    // namespace Board::detail::flash