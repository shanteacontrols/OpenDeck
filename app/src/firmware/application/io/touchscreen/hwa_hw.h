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
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        bool init() override
        {
            static constexpr uint32_t BAUDRATE = 38400;
            return board::uart::init(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, BAUDRATE) == board::initStatus_t::OK;
        }

        bool deInit() override
        {
            return board::uart::deInit(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN);
        }

        bool write(uint8_t value) override
        {
            return board::uart::write(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, value);
        }

        bool read(uint8_t& value) override
        {
            return board::uart::read(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN, value);
        }

        bool allocated(io::common::Allocatable::interface_t interface) override
        {
#ifdef PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN
            return board::uart::isInitialized(PROJECT_TARGET_UART_CHANNEL_TOUCHSCREEN);
#else
            return false;
#endif
        }
    };
}    // namespace io::touchscreen
