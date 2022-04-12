/*

Copyright 2015-2022 Igor Petrovic

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

#include "protocol/ProtocolBase.h"
#include "core/src/MCU.h"

namespace Protocol
{
    class DMX : public Protocol::Base
    {
        public:
        enum class setting_t : uint8_t
        {
            ENABLED,
            AMOUNT
        };

        class HWA : public DMXUSBWidget::HWA, public IO::Common::Allocatable
        {
            public:
            virtual bool uniqueID(core::mcu::uniqueID_t& uniqueID) = 0;
        };

        DMX(HWA& hwa, Database::Instance& database)
        {}

        bool init() override
        {
            return false;
        }

        bool deInit() override
        {
            return false;
        }

        void read() override
        {}
    };
}    // namespace Protocol