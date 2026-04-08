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

#include <gmock/gmock.h>

namespace io::analog
{
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        uint8_t adcBits() override
        {
            return 10;    // unused in tests
        }

        MOCK_METHOD2(value, bool(size_t index, uint16_t& value));
    };
}    // namespace io::analog