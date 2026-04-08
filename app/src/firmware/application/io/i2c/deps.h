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

#include <inttypes.h>
#include <stddef.h>

namespace io::i2c
{
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        virtual bool init()                                               = 0;
        virtual bool write(uint8_t address, uint8_t* buffer, size_t size) = 0;
        virtual bool deviceAvailable(uint8_t address)                     = 0;
    };

    class Peripheral
    {
        public:
        virtual ~Peripheral() = default;

        virtual bool init()   = 0;
        virtual void update() = 0;
    };
}    // namespace io::i2c