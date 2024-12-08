/*

Copyright Igor Petrovic

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

#include "board/board.h"
#include "internal.h"
#include <target.h>

#ifdef PROJECT_TARGET_SUPPORT_BOOTLOADER_BUTTON
#ifdef PROJECT_TARGET_BOOTLOADER_BUTTON_ACTIVE_HIGH
#define IS_BTLDR_ACTIVE() (CORE_MCU_IO_READ(PIN_PORT_BTLDR_BUTTON, PIN_INDEX_BTLDR_BUTTON))
#else
#define IS_BTLDR_ACTIVE() (!(CORE_MCU_IO_READ(PIN_PORT_BTLDR_BUTTON, PIN_INDEX_BTLDR_BUTTON)))
#endif
#else
#define IS_BTLDR_ACTIVE() (false)
#endif

// Holds total flash size.
// Inserted in the binary by build process - this is just the dummy definition.
// Address where this variable is stored contains total firmware length.
// After the last firmware address, CRC of firmware is stored.
// This is used by the bootloader to verify the CRC of application.
uint32_t flashSize __attribute__((section(".fwMetadata"))) __attribute__((used)) = 0;

namespace board::bootloader
{
    void appAddrBoundary(uint32_t& first, uint32_t& last)
    {
        first = PROJECT_MCU_FLASH_ADDR_APP_START;
        core::mcu::flash::read32(PROJECT_MCU_FLASH_ADDR_FW_METADATA_START, last);
    }

    bool isHwTriggerActive()
    {
        // add some delay before reading the pins to avoid incorrect state detection
        core::mcu::timing::waitMs(100);

        return IS_BTLDR_ACTIVE();
    }

    uint32_t pageSize(size_t index)
    {
        return core::mcu::flash::pageSize(index + PROJECT_MCU_FLASH_PAGE_APP);
    }

    void erasePage(size_t index)
    {
        core::mcu::bootloader::erasePage(index + PROJECT_MCU_FLASH_PAGE_APP);
    }

    void fillPage(size_t index, uint32_t address, uint32_t value)
    {
        core::mcu::bootloader::fillPage(index + PROJECT_MCU_FLASH_PAGE_APP, address, value);
    }

    void commitPage(size_t index)
    {
        core::mcu::bootloader::commitPage(index + PROJECT_MCU_FLASH_PAGE_APP);
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

    void runBootloader()
    {
        core::mcu::timers::startAll();
        board::usb::init();
        detail::io::indicators::indicateBootloaderLoad();
    }
}    // namespace board::bootloader