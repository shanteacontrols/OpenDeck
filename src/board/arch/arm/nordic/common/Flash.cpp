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
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "logger/Logger.h"
#include <Target.h>

NRF_FSTORAGE_DEF(nrf_fstorage_t _fstorage) = {
    .evt_handler = NULL,
    .start_addr  = FLASH_START_ADDR,
    .end_addr    = FLASH_END,
};

namespace Board::detail::flash
{
    bool init()
    {
        nrf_fstorage_api_t* fsAPI = &nrf_fstorage_sd;
        BOARD_ERROR_CHECK(nrf_fstorage_init(&_fstorage, fsAPI, NULL), NRF_SUCCESS);

        return true;
    }

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

    bool erasePage(size_t index)
    {
        BOARD_ERROR_CHECK(nrf_fstorage_erase(&_fstorage,
                                             detail::map::flashPageDescriptor(index).address,
                                             1,
                                             NULL),
                          NRF_SUCCESS);

        while (nrf_fstorage_is_busy(&_fstorage))
        {
            sd_app_evt_wait();
        }

        return true;
    }

    void writePage(size_t index)
    {
        // nothing to do here
    }

    bool write16(uint32_t address, uint16_t data)
    {
        BOARD_ERROR_CHECK(nrf_fstorage_write(&_fstorage,
                                             address,
                                             &data,
                                             sizeof(data),
                                             NULL),
                          NRF_SUCCESS);

        while (nrf_fstorage_is_busy(&_fstorage))
        {
            sd_app_evt_wait();
        }

        return (*(volatile uint16_t*)address) == data;
    }

    bool write32(uint32_t address, uint32_t data)
    {
        BOARD_ERROR_CHECK(nrf_fstorage_write(&_fstorage,
                                             address,
                                             &data,
                                             sizeof(data),
                                             NULL),
                          NRF_SUCCESS);

        while (nrf_fstorage_is_busy(&_fstorage))
        {
            sd_app_evt_wait();
        }

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