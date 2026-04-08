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

// Force the generated flash functions into test namespace to avoid clashes with
// stub functions (since stub MCU is used for tests)
namespace test
{
#include <core_mcu_generated.h>
}

namespace updater
{
    class HwaTest : public Hwa
    {
        public:
        uint32_t pageSize(size_t index) override
        {
            return test::core::mcu::flash::pageSize(index);
        }

        void erasePage(size_t index) override
        {
        }

        void fillPage(size_t index, uint32_t address, uint32_t value) override
        {
            _writtenBytes.push_back(value >> 0 & static_cast<uint32_t>(0xFF));
            _writtenBytes.push_back(value >> 8 & static_cast<uint32_t>(0xFF));
            _writtenBytes.push_back(value >> 16 & static_cast<uint32_t>(0xFF));
            _writtenBytes.push_back(value >> 24 & static_cast<uint32_t>(0xFF));
        }

        void commitPage(size_t index) override
        {
        }

        void apply() override
        {
            _updated = true;
        }

        void onFirmwareUpdateStart() override
        {
        }

        std::vector<uint8_t> _writtenBytes = {};
        bool                 _updated      = false;
    };
}    // namespace updater
