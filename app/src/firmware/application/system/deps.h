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

#include "common.h"
#include "bootloader/fw_selector/fw_selector.h"
#include "application/io/base.h"
#include "application/protocol/base.h"
#include "application/database/database.h"

#include <array>
#include <functional>

namespace sys
{
    using usbConnectionHandler_t = std::function<void()>;
    using ioComponents_t         = std::array<::io::Base*, static_cast<size_t>(::io::ioComponent_t::AMOUNT)>;
    using protocolComponents_t   = std::array<::protocol::Base*, static_cast<size_t>(::protocol::protocol_t::AMOUNT)>;

    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        virtual bool init()                                                                        = 0;
        virtual void update()                                                                      = 0;
        virtual void reboot(fw_selector::fwType_t type)                                            = 0;
        virtual void registerOnUSBconnectionHandler(usbConnectionHandler_t&& usbConnectionHandler) = 0;
    };

    class Components
    {
        public:
        virtual ~Components() = default;

        virtual ioComponents_t&       io()       = 0;
        virtual protocolComponents_t& protocol() = 0;
        virtual database::Admin&      database() = 0;
    };
}    // namespace sys