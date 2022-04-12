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
#include "board/common/io/Helpers.h"
#include "core/src/Timing.h"
#include <Target.h>

// Holds total flash size.
// Inserted in the binary by build process - this is just the dummy definition.
// Address where this variable is stored contains total firmware length.
// After the last firmware address, CRC of firmware is stored.
// This is used by the bootloader to verify the CRC of application.
uint32_t _flashSize __attribute__((section(".fwMetadata"))) __attribute__((used)) = 0;

namespace Board::bootloader
{
    void appAddrBoundary(uint32_t& first, uint32_t& last)
    {
        first = APP_START_ADDR;
        core::mcu::flash::read32(FW_METADATA_LOCATION, last);
    }

    bool isHWtriggerActive()
    {
        // add some delay before reading the pins to avoid incorrect state detection
        core::timing::waitMs(100);

#if defined(BTLDR_BUTTON_PORT)
#ifdef BTLDR_BUTTON_AH
        return CORE_MCU_IO_READ(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#else
        return !CORE_MCU_IO_READ(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#endif
#else
        // no hardware entry possible in this case
        return false;
#endif
    }

    uint32_t pageSize(size_t index)
    {
        return core::mcu::flash::pageSize(index + FLASH_PAGE_APP_START);
    }

    void erasePage(size_t index)
    {
        core::mcu::flash::erasePage(index + FLASH_PAGE_APP_START);
    }

    void fillPage(size_t index, uint32_t address, uint16_t value)
    {
        core::mcu::flash::write16(detail::map::flashPageDescriptor(index + FLASH_PAGE_APP_START).address + address, value);
    }

    void writePage(size_t index)
    {
        core::mcu::flash::writePage(index + FLASH_PAGE_APP_START);
    }

    uint8_t readFlash(uint32_t address)
    {
        uint8_t data = 0;

        if (!core::mcu::flash::read8(address, data))
        {
            data = 0;
        }

        return data;
    }
}    // namespace Board::bootloader