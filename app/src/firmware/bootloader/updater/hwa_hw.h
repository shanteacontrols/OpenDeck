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

namespace updater
{
    class HwaHw : public Hwa
    {
        public:
        uint32_t pageSize(size_t index) override
        {
            return board::bootloader::pageSize(index);
        }

        void erasePage(size_t index) override
        {
            board::bootloader::erasePage(index);
        }

        void fillPage(size_t index, uint32_t address, uint32_t value) override
        {
            board::bootloader::fillPage(index, address, value);
        }

        void commitPage(size_t index) override
        {
            board::bootloader::commitPage(index);
        }

        void apply() override
        {
            board::reboot();
        }

        void onFirmwareUpdateStart() override
        {
            board::io::indicators::indicateFirmwareUpdateStart();
        }
    };
}    // namespace updater
