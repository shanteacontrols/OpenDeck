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

#pragma once

#include "deps.h"
#include "board/board.h"

namespace fw_selector
{
    class HwaHw : public Hwa
    {
        public:
        uint32_t magicBootValue() override
        {
            return board::bootloader::magicBootValue();
        }

        void setMagicBootValue(uint32_t value) override
        {
            board::bootloader::setMagicBootValue(value);
        }

        void load(fwType_t fwType) override
        {
            switch (fwType)
            {
            case fwType_t::BOOTLOADER:
            {
                board::bootloader::runBootloader();
            }
            break;

            case fwType_t::APPLICATION:
            default:
            {
                board::bootloader::runApplication();
            }
            break;
            }
        }

        void appAddrBoundary(uint32_t& first, uint32_t& last) override
        {
            board::bootloader::appAddrBoundary(first, last);
        }

        bool isHwTriggerActive() override
        {
            return board::bootloader::isHwTriggerActive();
        }

        uint8_t readFlash(uint32_t address) override
        {
            return board::bootloader::readFlash(address);
        }
    };
}    // namespace fw_selector
