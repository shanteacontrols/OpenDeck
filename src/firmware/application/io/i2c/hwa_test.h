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

namespace io::i2c
{
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        bool init() override
        {
            return true;
        }

        bool write(uint8_t address, uint8_t* buffer, size_t size) override
        {
            return true;
        }

        bool deviceAvailable(uint8_t address) override
        {
            return false;
        }
    };
}    // namespace io::i2c