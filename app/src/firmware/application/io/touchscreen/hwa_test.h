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

#include "core/util/util.h"

namespace io::touchscreen
{
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        bool init() override
        {
            return true;
        }

        bool deInit() override
        {
            return true;
        }

        bool write(uint8_t value) override
        {
            return true;
        }

        bool read(uint8_t& value) override
        {
            return true;
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
            return false;
        }
    };
}    // namespace io::touchscreen
