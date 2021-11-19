/*

Copyright 2015-2021 Igor Petrovic

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

#include <array>
#include <stddef.h>
#include <inttypes.h>
#include "io/IOBase.h"

#ifdef I2C_SUPPORTED

namespace IO
{
    class I2C : public IO::Base
    {
        public:
        class Peripheral
        {
            public:
            virtual bool           init(uint8_t address)     = 0;
            virtual void           update()                  = 0;
            virtual const uint8_t* addresses(size_t& amount) = 0;
        };

        class HWA
        {
            public:
            virtual bool init()                                               = 0;
            virtual bool write(uint8_t address, uint8_t* buffer, size_t size) = 0;
            virtual bool deviceAvailable(uint8_t address)                     = 0;
        };

        I2C(HWA& hwa);

        bool        init() override;
        void        update(bool forceRefresh = false) override;
        static void registerPeripheral(Peripheral* instance);

        private:
        HWA&                                            _hwa;
        static constexpr size_t                         MAX_PERIPHERALS = 5;
        static size_t                                   _peripheralCounter;
        static std::array<Peripheral*, MAX_PERIPHERALS> _peripherals;
    };
}    // namespace IO

#else
#include "stub/I2C.h"
#endif