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

namespace io
{
    enum class ioComponent_t : uint8_t
    {
        BUTTONS,
        ENCODERS,
        ANALOG,
        LEDS,
        I2C,
        TOUCHSCREEN,
        AMOUNT
    };

    class Base
    {
        public:
        virtual ~Base() = default;

        virtual bool   init()                                                = 0;
        virtual void   updateSingle(size_t index, bool forceRefresh = false) = 0;
        virtual void   updateAll(bool forceRefresh = false)                  = 0;
        virtual size_t maxComponentUpdateIndex()                             = 0;
    };
}    // namespace io
