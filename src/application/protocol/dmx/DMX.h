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

#include "dmxusb/src/DMXUSBWidget.h"
#include "io/common/Common.h"
#include "system/Config.h"
#include "database/Database.h"
#include "protocol/ProtocolBase.h"

#ifdef DMX_SUPPORTED
namespace Protocol
{
    class DMX : public DMXUSBWidget, public Protocol::Base
    {
        public:
        enum class setting_t : uint8_t
        {
            enable,
            AMOUNT
        };

        using uniqueID_t = std::array<uint8_t, UID_BITS / 8>;

        class HWA : public IO::Common::Allocatable, public DMXUSBWidget::HWA
        {
            public:
            virtual bool uniqueID(uniqueID_t& uniqueID) = 0;
        };

        DMX(HWA& hwa, Database& database);

        bool init() override;
        bool deInit() override;
        void read() override;

        private:
        HWA&      _hwa;
        Database& _database;

        bool _enabled = false;

        std::optional<uint8_t> sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value);
    };
}    // namespace Protocol
#else
#include "stub/DMX.h"
#endif