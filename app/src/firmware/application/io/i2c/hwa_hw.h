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

namespace io::i2c
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        bool init() override
        {
            // for i2c, consider ALREADY_INIT status a success
            return board::i2c::init(PROJECT_TARGET_I2C_CHANNEL_DISPLAY, board::i2c::clockSpeed_t::S400K) != board::initStatus_t::ERROR;
        }

        bool write(uint8_t address, uint8_t* buffer, size_t size) override
        {
            return board::i2c::write(PROJECT_TARGET_I2C_CHANNEL_DISPLAY, address, buffer, size);
        }

        bool deviceAvailable(uint8_t address) override
        {
            return board::i2c::deviceAvailable(PROJECT_TARGET_I2C_CHANNEL_DISPLAY, address);
        }
    };
}    // namespace io::i2c