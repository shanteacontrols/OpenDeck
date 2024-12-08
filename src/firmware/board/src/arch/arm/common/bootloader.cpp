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

#include "core/mcu.h"

using appEntry_t = void (*)();

namespace
{
    /// Variable used to specify whether to enter bootloader or application.
    uint32_t fwEntryType __attribute__((section(".noinit"))) __attribute__((used));
}    // namespace

namespace board::bootloader
{
    uint32_t magicBootValue()
    {
        return fwEntryType;
    }

    void setMagicBootValue(uint32_t value)
    {
        fwEntryType = value;
    }

    void runApplication()
    {
        core::mcu::deInit();

        auto appEntry = (appEntry_t) * (volatile uint32_t*)(PROJECT_MCU_FLASH_ADDR_APP_START + PROJECT_MCU_FLASH_OFFSET_APP_JUMP_FROM_BOOTLOADER);
        appEntry();

        while (true)
        {
            ;
        }
    }
}    // namespace board::bootloader