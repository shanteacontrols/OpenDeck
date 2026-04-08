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

namespace io::touchscreen
{
    class HwaStub : public Hwa
    {
        public:
        HwaStub() = default;

        bool init() override
        {
            return false;
        }

        bool deInit() override
        {
            return false;
        }

        bool write(uint8_t value) override
        {
            return false;
        }

        bool read(uint8_t& value) override
        {
            return false;
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
            return false;
        }
    };
}    // namespace io::touchscreen
